/*
 * Copyright (C) 2022-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#ifndef STM32MP_DDR_H
#define STM32MP_DDR_H

#include <platform_def.h>

enum stm32mp_ddr_base_type {
	DDR_BASE,
	DDRPHY_BASE,
	NONE_BASE
};

enum stm32mp_ddr_reg_type {
	REG_REG,
	REG_TIMING,
	REG_PERF,
	REG_MAP,
	REGPHY_REG,
	REGPHY_TIMING,
	REG_TYPE_NB
};

struct stm32mp_ddr_reg_desc {
	uint16_t offset;	/* Offset for base address */
	uint8_t par_offset;	/* Offset for parameter array */
#ifdef STM32MP2X
	bool qd; /* quasi-dynamic register if true */
#endif
};

struct stm32mp_ddr_reg_info {
	const char *name;
	const struct stm32mp_ddr_reg_desc *desc;
	uint8_t size;
	enum stm32mp_ddr_base_type base;
};

struct stm32mp_ddr_size {
	uint64_t base;
	uint64_t size;
};

struct stm32mp_ddr_priv {
	struct stm32mp_ddr_size info;
	struct stm32mp_ddrctl *ctl;
	struct stm32mp_ddrphy *phy;
	uintptr_t pwr;
	uintptr_t rcc;
};

struct stm32mp_ddr_info {
	const char *name;
	uint32_t speed; /* in kHz */
	size_t size;    /* Memory size in byte = col * row * width */
};

#define DDR_DELAY_1US		1U
#define DDR_DELAY_2US		2U
#define DDR_DELAY_10US		10U
#define DDR_DELAY_50US		50U
#define DDR_TIMEOUT_500US	500U
#define DDR_TIMEOUT_US_1S	1000000U

void stm32mp_ddr_set_reg(const struct stm32mp_ddr_priv *priv, enum stm32mp_ddr_reg_type type,
			 const void *param, const struct stm32mp_ddr_reg_info *ddr_registers);
void stm32mp_ddr_start_sw_done(struct stm32mp_ddrctl *ctl);
void stm32mp_ddr_wait_sw_done_ack(struct stm32mp_ddrctl *ctl);
void stm32mp_ddr_enable_axi_port(struct stm32mp_ddrctl *ctl);
int stm32mp_ddr_disable_axi_port(struct stm32mp_ddrctl *ctl);
void stm32mp_ddr_enable_host_interface(struct stm32mp_ddrctl *ctl);
void stm32mp_ddr_disable_host_interface(struct stm32mp_ddrctl *ctl);
int stm32mp_ddr_sw_selfref_entry(struct stm32mp_ddrctl *ctl);
void stm32mp_ddr_sw_selfref_exit(struct stm32mp_ddrctl *ctl);
void stm32mp_ddr_set_qd3_update_conditions(struct stm32mp_ddrctl *ctl);
void stm32mp_ddr_unset_qd3_update_conditions(struct stm32mp_ddrctl *ctl);
void stm32mp_ddr_wait_refresh_update_done_ack(struct stm32mp_ddrctl *ctl);
int stm32mp_board_ddr_power_init(enum ddr_type ddr_type);

#endif /* STM32MP_DDR_H */
