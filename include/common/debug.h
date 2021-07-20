/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <lib/utils_def.h>

/*
 * The log output macros print output to the console. These macros produce
 * compiled log output only if the LOG_LEVEL defined in the makefile (or the
 * make command line) is greater or equal than the level required for that
 * type of log output.
 *
 * The format expected is the same as for printf(). For example:
 * INFO("Info %s.\n", "message")    -> INFO:    Info message.
 * WARN("Warning %s.\n", "message") -> WARNING: Warning message.
 */

#define LOG_LEVEL_NONE			U(0)
#define LOG_LEVEL_ERROR			U(10)
#define LOG_LEVEL_NOTICE		U(20)
#define LOG_LEVEL_WARNING		U(30)
#define LOG_LEVEL_INFO			U(40)
#define LOG_LEVEL_VERBOSE		U(50)

#ifndef __ASSEMBLER__

#include <cdefs.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include <drivers/console.h>

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

/*
 * If the log output is too low then this macro is used in place of tf_log()
 * below. The intent is to get the compiler to evaluate the function call for
 * type checking and format specifier correctness but let it optimize it out.
 */
#define no_tf_log(fmt, ...)				\
	do {						\
		if (false) {				\
			tf_log(fmt, ##__VA_ARGS__);	\
		}					\
	} while (false)

#if LOG_LEVEL >= LOG_LEVEL_ERROR
# define ERROR(...)	tf_log(LOG_MARKER_ERROR __VA_ARGS__)
# define ERROR_NL()	tf_log_newline(LOG_MARKER_ERROR)
#else
# define ERROR(...)	no_tf_log(LOG_MARKER_ERROR __VA_ARGS__)
# define ERROR_NL()
#endif

#if LOG_LEVEL >= LOG_LEVEL_NOTICE
# define NOTICE(...)	tf_log(LOG_MARKER_NOTICE __VA_ARGS__)
#else
# define NOTICE(...)	no_tf_log(LOG_MARKER_NOTICE __VA_ARGS__)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
# define WARN(...)	tf_log(LOG_MARKER_WARNING __VA_ARGS__)
#else
# define WARN(...)	no_tf_log(LOG_MARKER_WARNING __VA_ARGS__)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
# define INFO(...)	tf_log(LOG_MARKER_INFO __VA_ARGS__)
#else
# define INFO(...)	no_tf_log(LOG_MARKER_INFO __VA_ARGS__)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
# define VERBOSE(...)	tf_log(LOG_MARKER_VERBOSE __VA_ARGS__)
#else
# define VERBOSE(...)	no_tf_log(LOG_MARKER_VERBOSE __VA_ARGS__)
#endif

#if ENABLE_BACKTRACE
void backtrace(const char *cookie);
const char *get_el_str(unsigned int el);
#else
#define backtrace(x)
#endif

void __dead2 do_panic(void);

#define panic()				\
	do {				\
		backtrace(__func__);	\
		console_flush();	\
		do_panic();		\
	} while (false)

/* Function called when stack protection check code detects a corrupted stack */
void __dead2 __stack_chk_fail(void);

void tf_log(const char *fmt, ...) __printflike(1, 2);
void tf_log_newline(const char log_fmt[2]);
void tf_log_set_max_level(unsigned int log_level);

#endif /* __ASSEMBLER__ */
#endif /* DEBUG_H */
