/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/event_log/event_log.h>

/* Event Log data */
static uint8_t event_log[PLAT_ARM_EVENT_LOG_MAX_SIZE];

/* FVP table with platform specific image IDs, names and PCRs */
const event_log_metadata_t fvp_event_log_metadata[] = {
	{ FW_CONFIG_ID, FW_CONFIG_STRING, PCR_0 },
	{ TB_FW_CONFIG_ID, TB_FW_CONFIG_STRING, PCR_0 },
	{ BL2_IMAGE_ID, BL2_STRING, PCR_0 },
	{ INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

void bl1_plat_mboot_init(void)
{
	event_log_init(event_log, PLAT_ARM_EVENT_LOG_MAX_SIZE, 0U);
}

void bl1_plat_mboot_finish(void)
{
	/*
	 * ToDo: populate tb_fw_config with Event Log address, its maximum size
	 * and filled size
	 */
}
