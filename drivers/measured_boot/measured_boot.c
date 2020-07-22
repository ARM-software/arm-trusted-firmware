/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/measured_boot/measured_boot.h>

/*
 * Init Measured Boot driver
 *
 * Initialises Event Log.
 */
void measured_boot_init(void)
{
	event_log_init();
}

/*
 * Finish Measured Boot driver
 *
 * Finalises Event Log and dumps the records to the debug console.
 */
void measured_boot_finish(void)
{
	uint8_t *log_addr;
	size_t log_size;
	int rc;

	rc = event_log_finalise(&log_addr, &log_size);
	if (rc != 0) {
		panic();
	}

	dump_event_log(log_addr, log_size);
}
