/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <plat_arm.h>
#include <sgi_plat_config.h>
#include <sgi_ras.h>

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the platform configuration structure */
	plat_config_init();

	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
}

void bl31_platform_setup(void)
{
	arm_bl31_platform_setup();

#if RAS_EXTENSION
	sgi_ras_intr_handler_setup();
#endif
}
