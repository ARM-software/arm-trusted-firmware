/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

/* The log output macros print output to the console. These macros produce
 * compiled log output only if the LOG_LEVEL defined in the makefile (or the
 * make command line) is greater or equal than the level required for that
 * type of log output.
 * The format expected is the same as for printf(). For example:
 * INFO("Info %s.\n", "message")    -> INFO:    Info message.
 * WARN("Warning %s.\n", "message") -> WARNING: Warning message.
 */

#define LOG_LEVEL_NONE			0
#define LOG_LEVEL_ERROR			10
#define LOG_LEVEL_NOTICE		20
#define LOG_LEVEL_WARNING		30
#define LOG_LEVEL_INFO			40
#define LOG_LEVEL_VERBOSE		50

#ifndef __ASSEMBLY__
#include <stdio.h>

#if LOG_LEVEL >= LOG_LEVEL_NOTICE
# define NOTICE(...)	tf_printf("NOTICE:  " __VA_ARGS__)
#else
# define NOTICE(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
# define ERROR(...)	tf_printf("ERROR:   " __VA_ARGS__)
#else
# define ERROR(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
# define WARN(...)	tf_printf("WARNING: " __VA_ARGS__)
#else
# define WARN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
# define INFO(...)	tf_printf("INFO:    " __VA_ARGS__)
#else
# define INFO(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
# define VERBOSE(...)	tf_printf("VERBOSE: " __VA_ARGS__)
#else
# define VERBOSE(...)
#endif


void __dead2 do_panic(void);
#define panic()	do_panic()

/* Function called when stack protection check code detects a corrupted stack */
void __dead2 __stack_chk_fail(void);

void tf_printf(const char *fmt, ...) __printflike(1, 2);
int tf_snprintf(char *s, size_t n, const char *fmt, ...) __printflike(3, 4);

#endif /* __ASSEMBLY__ */
#endif /* __DEBUG_H__ */
