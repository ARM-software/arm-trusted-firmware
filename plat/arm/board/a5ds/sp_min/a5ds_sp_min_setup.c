/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

void plat_arm_sp_min_early_platform_setup(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	arm_sp_min_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
}

/*
 * A5DS will only have one always-on power domain and there
 * is no power control present.
 */
void plat_arm_pwrc_setup(void)
{
}

