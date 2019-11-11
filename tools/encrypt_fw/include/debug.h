/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

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


#if LOG_LEVEL >= LOG_LEVEL_NOTICE
# define NOTICE(...)	printf("NOTICE:  " __VA_ARGS__)
#else
# define NOTICE(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
# define ERROR(...)	printf("ERROR:   " __VA_ARGS__)
#else
# define ERROR(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
# define WARN(...)	printf("WARNING: " __VA_ARGS__)
#else
# define WARN(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
# define INFO(...)	printf("INFO:    " __VA_ARGS__)
#else
# define INFO(...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
# define VERBOSE(...)	printf("VERBOSE: " __VA_ARGS__)
#else
# define VERBOSE(...)
#endif

#endif /* DEBUG_H */
