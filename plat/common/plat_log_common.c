/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <platform.h>

/* Allow platforms to override the log prefix string */
#pragma weak plat_log_get_prefix

static const char *prefix_str[] = {
	"ERROR:   ", "NOTICE:  ", "WARNING: ", "INFO:    ", "VERBOSE: "};

const char *plat_log_get_prefix(unsigned int log_level)
{
	if (log_level < LOG_LEVEL_ERROR)
		log_level = LOG_LEVEL_ERROR;
	else if (log_level > LOG_LEVEL_VERBOSE)
		log_level = LOG_LEVEL_VERBOSE;

	return prefix_str[(log_level/10) - 1];
}
