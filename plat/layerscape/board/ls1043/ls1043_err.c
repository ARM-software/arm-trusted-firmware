/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>

/*
 * Error handler
 */
void plat_error_handler(int err)
{
	switch (err) {
	case -ENOENT:
	case -EAUTH:
		/* ToDo */
		break;
	default:
		/* Unexpected error */
		break;
	}

	/* Loop until the watchdog resets the system */
	for (;;)
		wfi();
}
