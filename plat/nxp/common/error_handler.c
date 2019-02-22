/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include <plat_common.h>

/*
 * Error handler
 */
void plat_error_handler(int err)
{

	switch (err) {
	case -ENOENT:
	case -EAUTH:
		break;
	default:
		/* Unexpected error */
		break;
	}

	/* Loop until the watchdog resets the system */
	for (;;)
		wfi();
}
