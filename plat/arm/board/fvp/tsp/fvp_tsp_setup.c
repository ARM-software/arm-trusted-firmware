/*
 * Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

#include "../fvp_private.h"

void tsp_early_platform_setup(u_register_t arg0, u_register_t arg1,
			      u_register_t arg2, u_register_t arg3)
{
	arm_tsp_early_platform_setup(arg0, arg1, arg2, arg3);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();
}
