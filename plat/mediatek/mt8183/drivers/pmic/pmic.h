/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMIC_H
#define PMIC_H

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

#endif /* PMIC_H */
