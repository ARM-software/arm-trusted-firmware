/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <lib/mmio.h>
#include <platform_def.h>

#include "spmi_common.h"
#include "spmi_sw.h"

/* SPMI Commands */
#define SPMI_CMD_EXT_WRITE			0x00
#define SPMI_CMD_EXT_READ			0x20
#define SPMI_CMD_EXT_WRITEL			0x30
#define SPMI_CMD_EXT_READL			0x38
#define SPMI_CMD_WRITE				0x40
#define SPMI_CMD_READ				0x60
#define SPMI_CMD_ZERO_WRITE			0x80
#define SPMI_READ_ADDR_MAX			0x1F

static struct spmi_device *spmi_dev[SPMI_MAX_SLAVE_ID];

int spmi_register_zero_write(struct spmi_device *dev, uint8_t data)
{
	return dev->pmif_arb->write_cmd(dev->pmif_arb, SPMI_CMD_ZERO_WRITE,
					dev->slvid, 0, &data, 1);
}

int spmi_register_read(struct spmi_device *dev, uint8_t addr, uint8_t *buf)
{
	/* 5-bit register address */
	if (addr > SPMI_READ_ADDR_MAX)
		return -EINVAL;

	return dev->pmif_arb->read_cmd(dev->pmif_arb, SPMI_CMD_READ, dev->slvid, addr, buf, 1);
}

int spmi_register_write(struct spmi_device *dev, uint8_t addr, uint8_t data)
{
	/* 5-bit register address */
	if (addr > SPMI_READ_ADDR_MAX)
		return -EINVAL;

	return dev->pmif_arb->write_cmd(dev->pmif_arb, SPMI_CMD_WRITE,
					dev->slvid, addr, &data, 1);
}

int spmi_ext_register_read(struct spmi_device *dev, uint8_t addr, uint8_t *buf,
			   uint8_t len)
{
	/* 8-bit register address, up to 16 bytes */
	if (len == 0 || len > 16)
		return -EINVAL;

	return dev->pmif_arb->read_cmd(dev->pmif_arb, SPMI_CMD_EXT_READ,
				       dev->slvid, addr, buf, len);
}

int spmi_ext_register_write(struct spmi_device *dev, uint8_t addr,
			    const uint8_t *buf, uint8_t len)
{
	/* 8-bit register address, up to 16 bytes */
	if (len == 0 || len > 16)
		return -EINVAL;

	return dev->pmif_arb->write_cmd(dev->pmif_arb, SPMI_CMD_EXT_WRITE,
					dev->slvid, addr, buf, len);
}

int spmi_ext_register_readl(struct spmi_device *dev, uint16_t addr,
			    uint8_t *buf, uint8_t len)
{
	/* 8-bit register address, up to 16 bytes */
	if (len == 0 || len > 16)
		return -EINVAL;

	return dev->pmif_arb->read_cmd(dev->pmif_arb, SPMI_CMD_EXT_READL,
				       dev->slvid, addr, buf, len);
}

int spmi_ext_register_writel(struct spmi_device *dev, uint16_t addr,
			     const uint8_t *buf, uint8_t len)
{
	/* 8-bit register address, up to 16 bytes */
	if (len == 0 || len > 16)
		return -EINVAL;

	return dev->pmif_arb->write_cmd(dev->pmif_arb, SPMI_CMD_EXT_WRITEL,
					dev->slvid, addr, buf, len);
}

int spmi_ext_register_readl_field(struct spmi_device *dev, uint16_t addr,
				  uint8_t *buf, uint16_t mask, uint16_t shift)
{
	int ret;
	uint8_t rdata = 0;

	ret = dev->pmif_arb->read_cmd(dev->pmif_arb, SPMI_CMD_EXT_READL,
				      dev->slvid, addr, &rdata, 1);
	if (!ret)
		*buf = (rdata >> shift) & mask;

	return ret;
}

int spmi_ext_register_writel_field(struct spmi_device *dev, uint16_t addr,
				   uint8_t data, uint16_t mask, uint16_t shift)
{
	int ret;
	uint8_t tmp = 0;

	ret = spmi_ext_register_readl(dev, addr, &tmp, 1);
	if (ret)
		return ret;

	tmp &= ~(mask << shift);
	tmp |= (data << shift);
	return dev->pmif_arb->write_cmd(dev->pmif_arb, SPMI_CMD_EXT_WRITEL,
					dev->slvid, addr, &tmp, 1);
}

struct spmi_device *get_spmi_device(int mstid, int slvid)
{
	if (slvid >= SPMI_MAX_SLAVE_ID || slvid < 0) {
		SPMI_ERR("failed to get spmi_device with slave id %d\n", slvid);
		return NULL;
	}
	return spmi_dev[slvid];
}

int spmi_device_register(struct spmi_device *platform_spmi_dev, unsigned int num_devs)
{
	int i;

	if (!platform_spmi_dev || num_devs == 0)
		return -EINVAL;

	for (i = 0; i < num_devs; i++) {
		if (platform_spmi_dev[i].slvid >= SPMI_MAX_SLAVE_ID ||
		    platform_spmi_dev[i].slvid < 0) {
			SPMI_INFO("invalid slave id %d\n", platform_spmi_dev[i].slvid);
			continue;
		}
		if (!spmi_dev[platform_spmi_dev[i].slvid])
			spmi_dev[platform_spmi_dev[i].slvid] = &platform_spmi_dev[i];
		else {
			SPMI_INFO("duplicated slave id %d\n", platform_spmi_dev[i].slvid);
			return -EINVAL;
		}
	}
	return 0;
}

static int spmi_ctrl_op_st(int mstid, unsigned int grpiden, unsigned int sid,
		unsigned int cmd)
{
	struct pmif *arb = get_pmif_controller(PMIF_SPMI, mstid);
	unsigned int rdata = 0x0;
	uintptr_t spmi_grp_id_en_addr =
		(uintptr_t)(arb->spmimst_base + arb->spmimst_regs[SPMI_GRP_ID_EN]);
	uintptr_t spmi_op_st_ctrl_addr =
		(uintptr_t)(arb->spmimst_base + arb->spmimst_regs[SPMI_OP_ST_CTRL]);
	uintptr_t spmi_op_st_sta_addr =
		(uintptr_t)(arb->spmimst_base + arb->spmimst_regs[SPMI_OP_ST_STA]);

	/* gid is 0x800 */
	mmio_write_32(spmi_grp_id_en_addr, grpiden);

	if (grpiden == (1 << SPMI_GROUP_ID))
		mmio_write_32(spmi_op_st_ctrl_addr, (cmd << 0x4) | SPMI_GROUP_ID);
	else
		mmio_write_32(spmi_op_st_ctrl_addr, (cmd << 0x4) | sid);

	SPMI_INFO("%s 0x%x\n", __func__, mmio_read_32(spmi_op_st_ctrl_addr));

	do {
		rdata = mmio_read_32(spmi_op_st_sta_addr);
		SPMI_INFO("%s 0x%x\n", __func__, rdata);

		if (((rdata >> 0x1) & SPMI_OP_ST_NACK) == SPMI_OP_ST_NACK) {
			SPMI_ERR("SPMI_OP_ST_NACK occurs! OP_ST_STA = 0x%x\n", rdata);
			break;
		}
	} while ((rdata & SPMI_OP_ST_BUSY) == SPMI_OP_ST_BUSY);

	return 0;
}

int spmi_command_shutdown(int mstid, struct spmi_device *dev, unsigned int grpiden)
{
	if (grpiden != (1 << SPMI_GROUP_ID))
		dev->slvid = grpiden;

	return spmi_ctrl_op_st(mstid, grpiden, dev->slvid, SPMI_SHUTDOWN);
}
