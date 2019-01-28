/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

#include <sgm_plat_config.h>

void tsp_early_platform_setup(void)
{
	/* Initialize the platform configuration structure */
	plat_config_init();

	arm_tsp_early_platform_setup();
}
