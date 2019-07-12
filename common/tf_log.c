/*
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <assert.h>
#include <stdio.h>

#include <common/debug.h>
#include <plat/common/platform.h>

/* Set the default maximum log level to the `LOG_LEVEL` build flag */
static unsigned int max_log_level = LOG_LEVEL;

/*
 * The common log function which is invoked by TF-A code.
 * This function should not be directly invoked and is meant to be
 * only used by the log macros defined in debug.h. The function
 * expects the first character in the format string to be one of the
 * LOG_MARKER_* macros defined in debug.h.
 */
void tf_log(const char *fmt, ...)
{
	unsigned int log_level;
	va_list args;
	const char *prefix_str;

	/* We expect the LOG_MARKER_* macro as the first character */
	log_level = fmt[0];

	/* Verify that log_level is one of LOG_MARKER_* macro defined in debug.h */
	assert((log_level > 0U) && (log_level <= LOG_LEVEL_VERBOSE));
	assert((log_level % 10U) == 0U);

	if (log_level > max_log_level)
		return;

	prefix_str = plat_log_get_prefix(log_level);

	while (*prefix_str != '\0') {
		(void)putchar(*prefix_str);
		prefix_str++;
	}

	va_start(args, fmt);
	(void)vprintf(fmt + 1, args);
	va_end(args);
}

/*
 * The helper function to set the log level dynamically by platform. The
 * maximum log level is determined by `LOG_LEVEL` build flag at compile time
 * and this helper can set a lower (or equal) log level than the one at compile.
 */
void tf_log_set_max_level(unsigned int log_level)
{
	assert(log_level <= LOG_LEVEL_VERBOSE);
	assert((log_level % 10U) == 0U);

	/* Cap log_level to the compile time maximum. */
	if (log_level <= (unsigned int)LOG_LEVEL)
		max_log_level = log_level;
}
