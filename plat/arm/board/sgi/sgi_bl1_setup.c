/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <plat_arm.h>
#include <sgi_plat_config.h>
#include <soc_css.h>

void bl1_early_platform_setup(void)
{
	/* Initialize the platform configuration structure */
	plat_config_init();

	arm_bl1_early_platform_setup();
}
