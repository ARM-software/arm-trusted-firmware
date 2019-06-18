/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
	u_register_t arg2, u_register_t arg3)
{
	arm_bl2_early_platform_setup((uintptr_t)arg0, (meminfo_t *)arg1);
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();
}
