/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

#include "fvp_private.h"

void bl2_el3_early_platform_setup(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
	arm_bl2_el3_early_platform_setup();

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();
	/*
	 * Enable coherency in Interconnect for the primary CPU's cluster.
	 */
	fvp_interconnect_enable();
}
