/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pmic.h>
#include <pmic_wrap_init.h>

void pmic_power_off(void)
{
	pwrap_write(PMIC_PWRHOLD, 0x0);
}
