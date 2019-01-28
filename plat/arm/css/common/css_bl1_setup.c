/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/soc/common/soc_css.h>
#include <plat/common/platform.h>

void bl1_platform_setup(void)
{
	arm_bl1_platform_setup();
	/*
	 * Do ARM CSS SoC security setup.
	 * BL1 needs to enable normal world access to memory.
	 */
	soc_css_security_setup();
}

