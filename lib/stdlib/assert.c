/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <console.h>
#include <debug.h>
#include <platform.h>

/*
* Only print the output if PLAT_LOG_LEVEL_ASSERT is higher or equal to
* LOG_LEVEL_INFO, which is the default value for builds with DEBUG=1.
*/

#if PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_VERBOSE
void __assert(const char *file, unsigned int line, const char *assertion)
{
	tf_printf("ASSERT: %s:%d:%s\n", file, line, assertion);
	console_flush();
	plat_panic_handler();
}
#elif PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_INFO
void __assert(const char *file, unsigned int line)
{
	tf_printf("ASSERT: %s:%d\n", file, line);
	console_flush();
	plat_panic_handler();
}
#else
void __assert(void)
{
	plat_panic_handler();
}
#endif
