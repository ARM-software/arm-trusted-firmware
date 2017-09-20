/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <platform.h>

/* Set the default maximum log level to the `LOG_LEVEL` build flag */
static unsigned int max_log_level = LOG_LEVEL;

/*
 * The common log function which is invoked by ARM Trusted Firmware code.
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
	assert(log_level && log_level <= LOG_LEVEL_VERBOSE);
	assert(log_level % 10 == 0);

	if (log_level > max_log_level)
		return;

	prefix_str = plat_log_get_prefix(log_level);

	if (prefix_str != NULL)
		tf_string_print(prefix_str);

	va_start(args, fmt);
	tf_vprintf(fmt+1, args);
	va_end(args);
}

/*
 * The helper function to set the log level dynamically by platform. The
 * maximum log level is determined by `LOG_LEVEL` build flag at compile time
 * and this helper can set a lower log level than the one at compile.
 */
void tf_log_set_max_level(unsigned int log_level)
{
	assert(log_level <= LOG_LEVEL_VERBOSE);
	assert((log_level % 10) == 0);

	/* Cap log_level to the compile time maximum. */
	if (log_level < LOG_LEVEL)
		max_log_level = log_level;

}
