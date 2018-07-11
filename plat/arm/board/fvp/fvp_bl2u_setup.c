/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_arm.h>
#include <platform.h>
#include "fvp_def.h"
#include "fvp_private.h"

void bl2u_early_platform_setup(struct meminfo *mem_layout, void *plat_info)
{
	arm_bl2u_early_platform_setup(mem_layout, plat_info);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();
}
