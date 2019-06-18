/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/*
 * We assume that all security programming is done by the primary core.
 */
void plat_arm_security_setup(void)
{
	/*
	 * The platform currently does not have any security setup.
	 */
}
