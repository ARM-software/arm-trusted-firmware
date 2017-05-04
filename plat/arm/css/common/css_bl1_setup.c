/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <plat_arm.h>
#include <soc_css.h>

void bl1_platform_setup(void)
{
	arm_bl1_platform_setup();
	/*
	 * Do ARM CSS SoC security setup.
	 * BL1 needs to enable normal world access to memory.
	 */
	soc_css_security_setup();
}

