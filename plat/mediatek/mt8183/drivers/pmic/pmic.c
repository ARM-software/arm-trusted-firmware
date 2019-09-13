/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pmic_wrap_init.h>
#include <pmic.h>

void wk_pmic_enable_sdn_delay(void)
{
	uint32_t con;

	pwrap_write(PMIC_TMA_KEY, 0x9CA7);
	pwrap_read(PMIC_PSEQ_ELR11, &con);
	con &= ~PMIC_RG_SDN_DLY_ENB;
	pwrap_write(PMIC_PSEQ_ELR11, con);
	pwrap_write(PMIC_TMA_KEY, 0);
}

void pmic_power_off(void)
{
	pwrap_write(PMIC_PWRHOLD, 0x0);
}
