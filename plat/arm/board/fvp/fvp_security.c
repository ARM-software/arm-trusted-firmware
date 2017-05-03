/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_config.h>
#include <plat_arm.h>

/*
 * We assume that all security programming is done by the primary core.
 */
void plat_arm_security_setup(void)
{
	/*
	 * The Base FVP has a TrustZone address space controller, the Foundation
	 * FVP does not. Trying to program the device on the foundation FVP will
	 * cause an abort.
	 *
	 * If the platform had additional peripheral specific security
	 * configurations, those would be configured here.
	 */

	if (get_arm_config()->flags & ARM_CONFIG_HAS_TZC)
		arm_tzc400_setup();
}
