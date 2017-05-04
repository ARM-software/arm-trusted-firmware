/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_arm.h>
#include "../fvp_private.h"

void tsp_early_platform_setup(void)
{
	arm_tsp_early_platform_setup();

	/* Initialize the platform config for future decision making */
	fvp_config_setup();
}
