/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <console.h>
#include <platform.h>
#include <platform_sp_min.h>

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that may be re-used by multiple
 * platforms but may also be overridden by a platform if required.
 */
#pragma weak sp_min_plat_runtime_setup

void sp_min_plat_runtime_setup(void)
{
	/*
	 * Finish the use of console driver in SP_MIN so that any runtime logs
	 * from SP_MIN will be suppressed.
	 */
	console_uninit();
}

#if !ERROR_DEPRECATED

#pragma weak sp_min_early_platform_setup2

void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	sp_min_early_platform_setup((void *)arg0, (void *)arg1);
}
#endif
