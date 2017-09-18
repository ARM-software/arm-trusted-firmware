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
#include <stdarg.h>
#include <stdio.h>

/*
 * Define Log Markers corresponding to each log level which will
 * be embedded in the format string and is expected by tf_log() to determine
 * the log level.
 */
#define LOG_MARKER_ERROR		"\xa"	/* 10 */
#define LOG_MARKER_NOTICE		"\x14"	/* 20 */
#define LOG_MARKER_WARNING		"\x1e"	/* 30 */
#define LOG_MARKER_INFO			"\x28"	/* 40 */
#define LOG_MARKER_VERBOSE		"\x32"	/* 50 */

#if LOG_LEVEL >= LOG_LEVEL_NOTICE
# define NOTICE(...)	tf_log(LOG_MARKER_NOTICE __VA_ARGS__)
#else
# define NOTICE(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
# define ERROR(...)	tf_log(LOG_MARKER_ERROR __VA_ARGS__)
#else
# define ERROR(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
# define WARN(...)	tf_log(LOG_MARKER_WARNING __VA_ARGS__)
#else
# define WARN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
# define INFO(...)	tf_log(LOG_MARKER_INFO __VA_ARGS__)
#else
# define INFO(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
# define VERBOSE(...)	tf_log(LOG_MARKER_VERBOSE __VA_ARGS__)
#else
# define VERBOSE(...)
#endif

void __dead2 do_panic(void);
#define panic()	do_panic()

/* Function called when stack protection check code detects a corrupted stack */
void __dead2 __stack_chk_fail(void);

void tf_log(const char *fmt, ...) __printflike(1, 2);
void tf_printf(const char *fmt, ...) __printflike(1, 2);
int tf_snprintf(char *s, size_t n, const char *fmt, ...) __printflike(3, 4);
void tf_vprintf(const char *fmt, va_list args);
void tf_string_print(const char *str);
void tf_log_set_max_level(unsigned int log_level);

#endif /* __ASSEMBLY__ */
#endif /* __DEBUG_H__ */
