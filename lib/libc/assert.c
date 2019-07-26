/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <stdio.h>

#include <common/debug.h>
#include <drivers/console.h>
#include <plat/common/platform.h>

/*
 * Only print the output if PLAT_LOG_LEVEL_ASSERT is higher or equal to
 * LOG_LEVEL_INFO, which is the default value for builds with DEBUG=1.
 */

#if PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_VERBOSE
void __dead2 __assert(const char *file, unsigned int line,
		      const char *assertion)
{
	printf("ASSERT: %s:%d:%s\n", file, line, assertion);
	backtrace("assert");
	(void)console_flush();
	plat_panic_handler();
}
#elif PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_INFO
void __dead2 __assert(const char *file, unsigned int line)
{
	printf("ASSERT: %s:%d\n", file, line);
	backtrace("assert");
	(void)console_flush();
	plat_panic_handler();
}
#else
void __dead2 __assert(void)
{
	backtrace("assert");
	(void)console_flush();
	plat_panic_handler();
}
#endif
