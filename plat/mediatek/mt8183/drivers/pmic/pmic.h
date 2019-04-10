/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PMIC_H__
#define __PMIC_H__

enum {
	PMIC_TMA_KEY = 0x03a8,
	PMIC_PWRHOLD = 0x0a08,
	PMIC_PSEQ_ELR11 = 0x0a62
};

enum {
	PMIC_RG_SDN_DLY_ENB = 1U << 10
};

/* external API */
void wk_pmic_enable_sdn_delay(void);
void pmic_power_off(void);

#endif /* __PMIC_H__ */
