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
#pragma weak bl2_plat_preload_setup
#pragma weak plat_try_next_boot_source

void __dead2 plat_error_handler(int err)
{
	while (1)
		wfi();
}

void bl2_plat_preload_setup(void)
{
}

int plat_try_next_boot_source(void)
{
	return 0;
}
