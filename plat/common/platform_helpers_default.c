/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <platform.h>

/*
 * Placeholder functions which can be redefined by each platfrom.
 */

#pragma weak plat_error_handler
#pragma weak bl1_plat_handle_pre_image_load
#pragma weak bl1_plat_handle_post_image_load
#pragma weak bl2_plat_preload_setup
#pragma weak bl2_plat_handle_pre_image_load
#pragma weak bl2_plat_handle_post_image_load
#pragma weak plat_try_next_boot_source

void __dead2 plat_error_handler(int err)
{
	while (1)
		wfi();
}

int bl1_plat_handle_pre_image_load(void)
{
	return 0;
}

int bl1_plat_handle_post_image_load(void)
{
	return 0;
}

void bl2_plat_preload_setup(void)
{
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	return 0;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return 0;
}

int plat_try_next_boot_source(void)
{
	return 0;
}
