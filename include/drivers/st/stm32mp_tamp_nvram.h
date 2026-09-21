/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_TAMP_NVRAM_H
#define STM32MP_TAMP_NVRAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <drivers/st/nvmem.h>
#include <dt-bindings/soc/rif.h>
#include <lib/utils_def.h>

#define TAMP_SECCFGR U(0x20)
#define TAMP_BKPRIFR(x) (U(0x70) + (U(0x4) * (U(x) - U(1))))
#define TAMP_RXCIDCFGR(x) (U(0x80) + (U(0x4) * (U(x))))
#define CURRENT_CID U(RIF_CID1)

#define REG_FIELD(_offset, _l, _h)                                 \
	{                                                       \
		.reg_offset = (_offset),				\
		.lowest_bit = (_l),				\
		.highest_bit = (_h)				\
	}

#define REG_FIELD_NULL	REG_FIELD(0U, 0U, 0U)

struct stm32_tamp_nvram_reg_field {
	size_t reg_offset;
	uint32_t lowest_bit;
	uint32_t highest_bit;
};

enum stm32_tamp_bkpreg_access {
	RONS,
	RNSWNS,
	RNSWS,
	RSWS,
	NORNOW,
};

struct stm32_tamp_nvram_drv_data {
	uintptr_t base;
	uintptr_t parent_base;
	size_t size;
	size_t parent_size;
	const void *tamp_fdt;
	int node;
	size_t nb_total_regs;
	int *bkpreg_zones_end;
	enum stm32_tamp_bkpreg_access *bkpreg_access;
	const struct stm32_tamp_nvram_cdata *cdata;
	bool *compartment_owner;
};

struct stm32_tamp_nvram_cdata {
	const unsigned int nb_zones;
	const struct stm32_tamp_nvram_reg_field *zone_cfg;
};

//Implemented for MP1 and MP2

enum stm32_tamp_bkpreg_access *
stm32_tamp_nvram_get_access_rights(struct stm32_tamp_nvram_drv_data *drv_data);

const struct stm32_tamp_nvram_cdata *stm32_tamp_nvram_get_cdata(void);

//Shared in core
int stm32_tamp_nvram_reg_field_write(uintptr_t base,
				     struct stm32_tamp_nvram_reg_field field,
				     uint32_t val);

int stm32_tamp_nvram_reg_field_read(uintptr_t base,
				    struct stm32_tamp_nvram_reg_field field,
				    uint32_t *val);

int stm32_tamp_nvram_reg_get_zone(struct stm32_tamp_nvram_drv_data *drv_data,
				  int reg);

#endif /* STM32MP_TAMP_NVRAM_H */
