/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>

#include <plat_arm.h>
#include <sgm_plat_config.h>
#include <soc_css.h>

void bl1_early_platform_setup(void)
{

	/* Initialize the console before anything else */
	arm_bl1_early_platform_setup();

	/* Initialize the platform configuration structure */
	plat_config_init();

#if !HW_ASSISTED_COHERENCY
	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_arm_interconnect_init();
	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_arm_interconnect_enter_coherency();
#endif
}
