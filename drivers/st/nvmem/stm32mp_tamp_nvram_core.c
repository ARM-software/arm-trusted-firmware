/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <libfdt.h>

#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/nvmem.h>
#include <drivers/st/stm32mp_tamp_nvram.h>
#include <lib/mmio.h>

#include <platform_def.h>
#include <stm32mp_dt.h>

#define DEBUG_RIGHT_MAX_SIZE 100

static struct stm32_tamp_nvram_drv_data nvram_drv_data;

int stm32_tamp_nvram_reg_get_zone(struct stm32_tamp_nvram_drv_data *drv_data, int reg)
{
	int *bkpreg_zones_end = drv_data->bkpreg_zones_end;
	unsigned int nb_zones = drv_data->cdata->nb_zones;
	unsigned int protection_zone_idx;

	for (protection_zone_idx = 0; protection_zone_idx < nb_zones; protection_zone_idx++) {
		if (reg <= bkpreg_zones_end[protection_zone_idx]) {
			break;
		}
	}

	if (protection_zone_idx >= nb_zones) {
		return -1; /* the reg is not a part of any zone */
	}

	return protection_zone_idx;
}

static int
stm32_tamp_nvram_read_bkp(struct stm32_tamp_nvram_drv_data *drv_data, size_t offset, uint32_t *val)
{
	unsigned int reg = (offset / sizeof(uint32_t));
	int protection_zone_idx = stm32_tamp_nvram_reg_get_zone(drv_data, (int)reg);
	int ret = -1;

	if (protection_zone_idx >= 0) {
		bool clk_should_be_toggled = !clk_is_enabled(TAMP_BKP_REG_CLK);

		switch (drv_data->bkpreg_access[protection_zone_idx]) {
		case RNSWNS:
		case RNSWS:
		case RONS:
		case RSWS:
			if (clk_should_be_toggled) {
				clk_enable(TAMP_BKP_REG_CLK);
			}
			*val = mmio_read_32(drv_data->base + offset);
			if (clk_should_be_toggled) {
				clk_disable(TAMP_BKP_REG_CLK);
			}
			ret = 0;
			break;
		case NORNOW:
			*val = U(0x0);
			ret = 0;
			break;
		default:
			ERROR("Wrong access rights for zone %d (%u)\n", protection_zone_idx, reg);
		}
	}

	return ret;
}

static int
stm32_tamp_nvram_write_bkp(struct stm32_tamp_nvram_drv_data *drv_data, size_t offset, uint32_t val)
{
	unsigned int reg = (offset / sizeof(uint32_t));
	int protection_zone_idx = stm32_tamp_nvram_reg_get_zone(drv_data, (int)reg);
	int ret = -1;

	if (protection_zone_idx >= 0) {
		bool clk_should_be_toggled = !clk_is_enabled(TAMP_BKP_REG_CLK);

		switch (drv_data->bkpreg_access[protection_zone_idx]) {
		case RNSWNS:
		case RNSWS:
		case RSWS:
			if (clk_should_be_toggled) {
				clk_enable(TAMP_BKP_REG_CLK);
			}
			mmio_write_32(drv_data->base + offset, val);
			if (clk_should_be_toggled) {
				clk_disable(TAMP_BKP_REG_CLK);
			}
			ret = 0;
			break;
		case RONS:
		case NORNOW:
			ret = 0;
			break;
		default:
			ERROR("Wrong access rights for zone %d (%u)\n", protection_zone_idx, reg);
		}
	}

	return ret;
}

int stm32_tamp_nvram_reg_field_read(uintptr_t base, struct stm32_tamp_nvram_reg_field field,
				    uint32_t *val)
{
	uint32_t reg_val = 0;

	reg_val = mmio_read_32(base + field.reg_offset);
	reg_val &= GENMASK_32(field.highest_bit, field.lowest_bit);
	reg_val >>= field.lowest_bit;
	*val = reg_val;

	return 0;
}

int stm32_tamp_nvram_reg_field_write(uintptr_t base, struct stm32_tamp_nvram_reg_field field,
				     uint32_t val)
{
	uint32_t reg_val = 0;
	uint32_t val_temp = val;
	bool clk_should_be_toggled = !clk_is_enabled(TAMP_BKP_REG_CLK);

	val_temp &= GENMASK_32(field.highest_bit - field.lowest_bit, 0);
	if (clk_should_be_toggled) {
		clk_enable(TAMP_BKP_REG_CLK);
	}
	reg_val = mmio_read_32(base + field.reg_offset);
	reg_val &= ~(GENMASK_32(field.highest_bit, field.lowest_bit));
	reg_val |= (val_temp << field.lowest_bit);
	mmio_write_32(base + field.reg_offset, reg_val);
	if (clk_should_be_toggled) {
		clk_disable(TAMP_BKP_REG_CLK);
	}

	return 0;
}

static int
stm32_tamp_nvram_write_byte(struct stm32_tamp_nvram_drv_data *drv_data, size_t offset, uint8_t byte)
{
	size_t offset_aligned = round_down(offset, sizeof(uint32_t));
	size_t byte_in_word = offset - offset_aligned;
	uint32_t read_value = 0;
	uint32_t to_be_writen_value = 0;
	uint32_t mask = UCHAR_MAX;

	mask = ~(mask << (byte_in_word * U(8)));

	stm32_tamp_nvram_read_bkp(drv_data, offset_aligned, &read_value);

	to_be_writen_value = read_value & mask;
	to_be_writen_value |= (uint32_t)byte << (byte_in_word * U(8));

	return stm32_tamp_nvram_write_bkp(drv_data, offset_aligned, to_be_writen_value);
}

static int
stm32_tamp_nvram_read_byte(struct stm32_tamp_nvram_drv_data *drv_data, size_t offset, uint8_t *byte)
{
	size_t offset_aligned = round_down(offset, sizeof(uint32_t));
	size_t byte_in_word = offset - offset_aligned;
	uint32_t read_value = 0;

	stm32_tamp_nvram_read_bkp(drv_data, offset_aligned, &read_value);
	*byte = (uint8_t)((read_value >> (byte_in_word * U(8))) & UCHAR_MAX);

	return 0;
}

static int
stm32_tamp_nvram_read(struct stm32_tamp_nvram_drv_data *drv_data, size_t offset, uint8_t *buf_u8,
		      size_t size)
{
	size_t i = offset;
	size_t total = offset + size;

	while (i < total) {
		size_t reg_idx = i / sizeof(uint32_t);

		if (((i + sizeof(uint32_t)) <= total) && is_aligned(i, sizeof(uint32_t))) {
			uint32_t tmp_u32 = 0;
			void *mem_cpy_ret = NULL;

			stm32_tamp_nvram_read_bkp(drv_data, i, &tmp_u32);
			mem_cpy_ret = memcpy(&buf_u8[i - offset],
					     (uint8_t *)&tmp_u32,
					     sizeof(uint32_t));
			if (mem_cpy_ret != &buf_u8[i - offset]) {
				ERROR("Failed to read bkp %zu\n", reg_idx);

				return -1;
			}
			i += sizeof(uint32_t);
		} else {
			uint8_t byte = 0;
			int ret = stm32_tamp_nvram_read_byte(drv_data, i, &byte);

			if (ret != 0) {
				ERROR("BKPreg %zu is not allowed to be read\n", reg_idx);
				byte = 0;
			}
			buf_u8[i - offset] = byte;
			i++;
		}
	}

	return 0;
}

static int
stm32_tamp_nvram_write(struct stm32_tamp_nvram_drv_data *drv_data, size_t offset,
		       const uint8_t *buf_u8, size_t size)
{
	size_t total = offset + size;
	size_t i = offset;

	while (i < total) {
		int ret = 0;
		size_t reg_idx = i / sizeof(uint32_t);

		if (((i + sizeof(uint32_t)) <= total) && is_aligned(i, sizeof(uint32_t))) {
			uint32_t tmp_u32 = 0;
			void *mem_cpy_ret = NULL;

			mem_cpy_ret = memcpy((uint8_t *)&tmp_u32,
					     &buf_u8[i - offset],
					     sizeof(uint32_t));
			if (mem_cpy_ret != &tmp_u32) {
				ERROR("Failed to read bkp %zu\n", reg_idx);
				return -1;
			}

			ret = stm32_tamp_nvram_write_bkp(drv_data, i, tmp_u32);
			if (ret != 0) {
				ERROR("Failed to read bkp %zu\n", reg_idx);
				return ret;
			}
			i += sizeof(uint32_t);
		} else {
			ret = stm32_tamp_nvram_write_byte(drv_data, i, buf_u8[i - offset]);
			if (ret != 0) {
				VERBOSE("BKPreg %zu is not allowed to be written\n", reg_idx);
			}
			i++;
		}
	}

	return 0;
}

static int *
stm32_tamp_nvram_get_backup_zones(struct stm32_tamp_nvram_drv_data *drv_data)
{
	static int static_bkpreg_zones_end[MAX_TAMP_BACKUP_REGS_ZONES];
	unsigned int nb_zones = drv_data->cdata->nb_zones;
	const struct stm32_tamp_nvram_cdata *cdata = drv_data->cdata;
	unsigned int zone_idx = 0;
	int *bkpreg_zones_end = static_bkpreg_zones_end;
	uint32_t offset_field = 0;
	struct stm32_tamp_nvram_reg_field zone_cfg_field = REG_FIELD_NULL;

	/* Get the n-1 frontiers of zone within the tamp configuration registers */
	for (zone_idx = 0; zone_idx < (nb_zones - U(1)); zone_idx++) {
		zone_cfg_field = cdata->zone_cfg[zone_idx];

		if (stm32_tamp_nvram_reg_field_read(drv_data->parent_base,
						    zone_cfg_field,
						    &offset_field) != 0) {
			ERROR("Can't read field for registers zones\n");
			return NULL;
		}

		/* in order to deal with no offset for a zone we store -1 for the zone end
		 * => the zone is empty
		 */
		bkpreg_zones_end[zone_idx] = ((int)offset_field) - 1;
	}

	/* The last zone end is defined by the number of registers in TAMP */
	bkpreg_zones_end[zone_idx] = ((int)drv_data->nb_total_regs) - 1;

	return bkpreg_zones_end;
}

static int
stm32_tamp_nvram_read_cell(struct nvmem_cell *cell, uint8_t *output_buf, size_t buf_len,
			   size_t *read_len)
{
	struct stm32_tamp_nvram_drv_data *drv_data =
		(struct stm32_tamp_nvram_drv_data *)cell->dev->drv_data;

	if (buf_len < cell->len) {
		return -1;
	}

	if (read_len != NULL) {
		*read_len = cell->len;
	}

	return stm32_tamp_nvram_read(drv_data, (size_t)cell->offset, output_buf, cell->len);
}

static int stm32_tamp_nvram_write_cell(struct nvmem_cell *cell, uint8_t *input_buf, size_t len)
{
	struct stm32_tamp_nvram_drv_data *drv_data =
		(struct stm32_tamp_nvram_drv_data *)cell->dev->drv_data;

	if (len > cell->len) {
		return -1;
	}

	return stm32_tamp_nvram_write(drv_data, (size_t)cell->offset, input_buf, cell->len);
}

static const struct nvmem_ops stm32_tamp_nvram_nvmem_ops = {
	.read_cell = stm32_tamp_nvram_read_cell,
	.write_cell = stm32_tamp_nvram_write_cell,
};

int stm32_tamp_nvram_init(void)
{
	int node = 0;
	int parent_node = 0;
	void *fdt = NULL;
	int len = 0;
	int ret;
	const fdt32_t *reg_prop = NULL;
	const fdt32_t *parent_reg_prop = NULL;

	if (fdt_get_address(&fdt) == 0) {
		return -ENODEV;
	}

	/* Get node and compatible data */
	node = fdt_node_offset_by_compatible(fdt, -1, DT_TAMP_NVRAM_COMPAT);
	if (node < 0) {
		return -ENODEV;
	}

	nvram_drv_data.cdata = stm32_tamp_nvram_get_cdata();

	reg_prop = fdt_getprop(fdt, node, "reg", &len);

	if ((reg_prop == NULL) || ((unsigned int)len != (sizeof(uint32_t) * U(2)))) {
		return -EINVAL;
	}

	nvram_drv_data.base = fdt32_to_cpu(reg_prop[0]);
	nvram_drv_data.size = fdt32_to_cpu(reg_prop[1]);

	parent_node = fdt_parent_offset(fdt, node);
	if (parent_node < 0) {
		return -EINVAL;
	}

	/* Get address of the device */
	parent_reg_prop = fdt_getprop(fdt, parent_node, "reg", &len);

	if ((parent_reg_prop == NULL) || ((unsigned int)len != (sizeof(uint32_t) * U(2)))) {
		return -EINVAL;
	}

	nvram_drv_data.parent_base = fdt32_to_cpu(parent_reg_prop[0]);
	nvram_drv_data.parent_size = fdt32_to_cpu(parent_reg_prop[1]);

	nvram_drv_data.nb_total_regs = nvram_drv_data.size / sizeof(uint32_t);

	ret = stm32_tamp_nvram_update_rights();
	if (ret != 0) {
		return ret;
	}

	nvram_drv_data.node = node;
	nvram_drv_data.tamp_fdt = fdt;

	nvmem_driver_register(nvram_drv_data.node, &stm32_tamp_nvram_nvmem_ops, &nvram_drv_data);

	return 0;
}

int stm32_tamp_nvram_update_rights(void)
{
	bool clk_should_be_toggled = !clk_is_enabled(TAMP_BKP_REG_CLK);

	if (clk_should_be_toggled) {
		clk_enable(TAMP_BKP_REG_CLK);
	}
	nvram_drv_data.bkpreg_zones_end = stm32_tamp_nvram_get_backup_zones(&nvram_drv_data);
	if (nvram_drv_data.bkpreg_zones_end == NULL) {
		NOTICE("Failed to get the backup zone from tamp regs\n\n");
		return -ENOMEM;
	}

	nvram_drv_data.bkpreg_access = stm32_tamp_nvram_get_access_rights(&nvram_drv_data);
	if (nvram_drv_data.bkpreg_access == NULL) {
		ERROR("Failed to get the backup zone access rights\n\n");
		return -ENOMEM;
	}

	if (clk_should_be_toggled) {
		clk_disable(TAMP_BKP_REG_CLK);
	}

	return 0;
}
