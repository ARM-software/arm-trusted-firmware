/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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

	return;
}
