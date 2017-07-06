/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#define LOG_LEVEL_NONE			0
#define LOG_LEVEL_ERROR			10
#define LOG_LEVEL_NOTICE		20
#define LOG_LEVEL_WARNING		30
#define LOG_LEVEL_INFO			40
#define LOG_LEVEL_VERBOSE		50

#ifndef __ASSEMBLY__
#include <platform_def.h>
#ifdef PLAT_LOG_MACROS
#include <plat_log_macros.h>
#else
#include <default_log_macros.h>
#endif
#include <stdio.h>

void __dead2 do_panic(void);
#define panic()	do_panic()

/* Function called when stack protection check code detects a corrupted stack */
void __dead2 __stack_chk_fail(void);

void tf_printf(const char *fmt, ...) __printflike(1, 2);
int tf_snprintf(char *s, size_t n, const char *fmt, ...) __printflike(3, 4);

#endif /* __ASSEMBLY__ */
#endif /* __DEBUG_H__ */
