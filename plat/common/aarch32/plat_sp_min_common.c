/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/console.h>
#include <plat/common/platform.h>
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
#if MULTI_CONSOLE_API
	console_switch_state(CONSOLE_FLAG_RUNTIME);
#else
	console_uninit();
#endif
}
