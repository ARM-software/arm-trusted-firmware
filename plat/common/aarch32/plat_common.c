/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <console.h>
#include <platform.h>
#include <xlat_mmu_helpers.h>

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that may be re-used by multiple
 * platforms but may also be overridden by a platform if required.
 */
#pragma weak bl32_plat_enable_mmu
#pragma weak sp_min_plat_runtime_setup

void bl32_plat_enable_mmu(uint32_t flags)
{
	enable_mmu_secure(flags);
}

void sp_min_plat_runtime_setup(void)
{
	/*
	 * Finish the use of console driver in SP_MIN so that any runtime logs
	 * from SP_MIN will be suppressed.
	 */
	console_uninit();
}
