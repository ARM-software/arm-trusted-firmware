/*
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <setjmp.h>
extern "C" {
#include <common/debug.h>
}

#include "CppUTestExt/MockSupport.h"
#include "common/mock_debug.h"

/* Set the default maximum log level to the `LOG_LEVEL` build flag */
static unsigned int max_log_level = LOG_LEVEL_VERBOSE;

int expect_panic(panic_environment_t *env) {
	mock("debug").expectOneCall("do_panic").andReturnValue(env);
	return 1;
}

void expect_tf_log(const char *str) {
	mock().expectOneCall("tf_log").withStringParameter("str", str);
}

extern "C" {
void do_panic(void) {
	panic_environment_t *env = (panic_environment_t *)mock("debug").actualCall("do_panic").returnPointerValue();
	longjmp(*env, 1);
}

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

	char log_buffer[1024];
	va_start(args, fmt);
	(void)vsnprintf(log_buffer, sizeof(log_buffer), fmt + 1, args);
	va_end(args);

	mock().actualCall("tf_log").withStringParameter("str", log_buffer);
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
	if (log_level <= (unsigned int)LOG_LEVEL_VERBOSE)
		max_log_level = log_level;
}
} // extern "C"

