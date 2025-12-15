/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * We assume that all security programming is done by the primary core.
 */
void plat_arm_security_setup(void)
{
	/*
	 * The Base FVP has a TrustZone address space controller.
	 *
	 * If the platform had additional peripheral specific security
	 * configurations, those would be configured here.
	 */

	return;
}
