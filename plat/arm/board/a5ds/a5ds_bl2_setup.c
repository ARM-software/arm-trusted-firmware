/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
	u_register_t arg2, u_register_t arg3)
{
	arm_bl2_early_platform_setup(arg0, arg1, arg2, arg3);
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return arm_bl2_plat_handle_post_image_load(image_id);
}
