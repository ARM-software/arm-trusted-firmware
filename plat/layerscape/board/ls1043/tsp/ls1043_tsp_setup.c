/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_arm.h>

void tsp_early_platform_setup(void)
{
	arm_tsp_early_platform_setup();

	/* Initialize the platform config for future decision making */
//	ls1043_config_setup();
}
