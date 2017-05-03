/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <console.h>
#include <debug.h>
#include <platform.h>

void __assert(const char *function, const char *file, unsigned int line,
		const char *assertion)
{
#if LOG_LEVEL >= LOG_LEVEL_INFO
	/*
	 * Only print the output if LOG_LEVEL is higher or equal to
	 * LOG_LEVEL_INFO, which is the default value for builds with DEBUG=1.
	 */
	tf_printf("ASSERT: %s <%d> : %s\n", function, line, assertion);

	console_flush();
#endif

	plat_panic_handler();
}
