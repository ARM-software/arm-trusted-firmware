/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_PSC_H
#define PMIC_PSC_H

#include <stdint.h>

enum pmic_psc_reg_name {
	RG_PWRHOLD,
	RG_CRST,
	RG_SMART_RST_SDN_EN,
	RG_SMART_RST_MODE,
};

struct pmic_psc_reg {
	uint16_t reg_addr;
	uint16_t reg_mask;
	uint16_t reg_shift;
};

struct pmic_psc_config {
	int (*read_field)(uint32_t reg, uint32_t *val, uint32_t mask, uint32_t shift);
	int (*write_field)(uint32_t reg, uint32_t val, uint32_t mask, uint32_t shift);
	const struct pmic_psc_reg *regs;
	const uint32_t reg_size;
};

#define PMIC_PSC_REG(_reg_name, addr, shift) \
	[_reg_name] = {			\
		.reg_addr = addr,	\
		.reg_mask = 0x1,	\
		.reg_shift = shift,	\
	}

int enable_pmic_smart_reset(bool enable);
int enable_pmic_smart_reset_shutdown(bool enable);
int platform_cold_reset(void);
int platform_power_hold(bool hold);
int pmic_psc_register(const struct pmic_psc_config *psc);

#endif /* PMIC_PSC_H */
