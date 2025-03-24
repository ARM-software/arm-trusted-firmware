/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/scu.h>
#include <plat/arm/common/plat_arm.h>


void plat_arm_sp_min_early_platform_setup(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	arm_sp_min_early_platform_setup(arg0, arg1, arg2, arg3);

	/* enable snoop control unit */
	enable_snoop_ctrl_unit(A5DS_SCU_BASE);
}

/*
 * A5DS will only have one always-on power domain and there
 * is no power control present.
 */
void plat_arm_pwrc_setup(void)
{
}

