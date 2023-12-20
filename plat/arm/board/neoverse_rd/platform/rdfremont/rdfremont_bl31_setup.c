/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <nrd_plat.h>


void bl31_platform_setup(void)
{
	nrd_bl31_common_platform_setup();
}
