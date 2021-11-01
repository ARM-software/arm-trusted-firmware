/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "fvp_private.h"

void bl2u_early_platform_setup(struct meminfo *mem_layout, void *plat_info)
{
	arm_bl2u_early_platform_setup(mem_layout, plat_info);

	/* Initialize System level generic or SP804 timer */
	fvp_timer_init();

	/* Initialize the platform config for future decision making */
	fvp_config_setup();
}
