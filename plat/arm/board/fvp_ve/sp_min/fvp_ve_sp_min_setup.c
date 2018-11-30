/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <plat/arm/common/plat_arm.h>

void plat_arm_sp_min_early_platform_setup(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	arm_sp_min_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
}
