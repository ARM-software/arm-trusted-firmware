/*
 * Copyright (c) 2019-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

void plat_arm_sp_min_early_platform_setup(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	arm_sp_min_early_platform_setup(arg0, arg1, arg2, arg3);
}
