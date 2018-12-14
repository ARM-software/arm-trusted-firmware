/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ASSERT_H
#define ASSERT_H

#include <cdefs.h>

#include <platform_def.h>

#include <common/debug.h>

#ifndef PLAT_LOG_LEVEL_ASSERT
#define PLAT_LOG_LEVEL_ASSERT	LOG_LEVEL
#endif

#if ENABLE_ASSERTIONS
# if PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_VERBOSE
#  define assert(e)	((e) ? (void)0 : __assert(__FILE__, __LINE__, #e))
# elif PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_INFO
#  define assert(e)	((e) ? (void)0 : __assert(__FILE__, __LINE__))
# else
#  define assert(e)	((e) ? (void)0 : __assert())
# endif
#else
#define assert(e)	((void)0)
#endif /* ENABLE_ASSERTIONS */

#if PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_VERBOSE
__dead2 void __assert(const char *file, unsigned int line,
		      const char *assertion);
#elif PLAT_LOG_LEVEL_ASSERT >= LOG_LEVEL_INFO
__dead2 void __assert(const char *file, unsigned int line);
#else
__dead2 void __assert(void);
#endif

#endif /* ASSERT_H */
