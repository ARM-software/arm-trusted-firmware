/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define L0_RESET 0x2

/*
 * Brcm error handler
 */
void plat_error_handler(int err)
{
	INFO("L0 reset...\n");

	/* Ensure the characters are flushed out */
	console_flush();

	mmio_write_32(CRMU_SOFT_RESET_CTRL, L0_RESET);

	/*
	 * In case we get here:
	 * Loop until the watchdog resets the system
	 */
	while (1) {
		wfi();
	}
}
