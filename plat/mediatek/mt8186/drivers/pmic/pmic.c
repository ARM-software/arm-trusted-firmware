/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <pmic.h>
#include <pmic_wrap_init.h>

uint32_t pmic_get_hwcid(void)
{
	uint32_t val = 0;

	pwrap_read(PMIC_RG_HWCID_ADDR, &val);

	return val;
}

void pmic_power_off(void)
{
	pwrap_write(PMIC_PWRHOLD, 0x0);
}
