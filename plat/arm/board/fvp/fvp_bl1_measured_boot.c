/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/metadata.h>
#include <tools_share/zero_oid.h>

#include <plat/arm/common/plat_arm.h>

/* Event Log data */
#if TRANSFER_LIST
static uint8_t *event_log;
#else
static uint8_t event_log[PLAT_ARM_EVENT_LOG_MAX_SIZE];
#endif

/* FVP table with platform specific image IDs, names and PCRs */
const event_log_metadata_t fvp_event_log_metadata[] = {
	{ FW_CONFIG_ID, MBOOT_FW_CONFIG_STRING, PCR_0 },
	{ TB_FW_CONFIG_ID, MBOOT_TB_FW_CONFIG_STRING, PCR_0 },
	{ BL2_IMAGE_ID, MBOOT_BL2_IMAGE_STRING, PCR_0 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) } /* Terminator */
};

void bl1_plat_mboot_init(void)
{
	size_t event_log_max_size;
	int rc;

#if TRANSFER_LIST
	event_log = transfer_list_event_log_extend(
		secure_tl, PLAT_ARM_EVENT_LOG_MAX_SIZE,
		&event_log_max_size);
	assert(event_log != NULL);
#else
	event_log_max_size = sizeof(event_log);
#endif

	rc = event_log_init(event_log, event_log + event_log_max_size);
	if (rc < 0) {
		ERROR("Failed to initialize event log (%d).\n", rc);
		panic();
	}

	rc = event_log_write_header();
	if (rc < 0) {
		ERROR("Failed to write event log header (%d).\n", rc);
		panic();
	}
}

void bl1_plat_mboot_finish(void)
{
	size_t event_log_cur_size = event_log_get_cur_size(event_log);

#if TRANSFER_LIST
	uint8_t *rc = transfer_list_event_log_finish(
		secure_tl, (uintptr_t)event_log + event_log_cur_size);

	if (rc != NULL) {
		return;
	}
#else
	int rc = arm_set_tb_fw_info((uintptr_t)event_log, event_log_cur_size,
				PLAT_ARM_EVENT_LOG_MAX_SIZE);
	if (rc == 0) {
		return;
	}
#endif

	/*
	 * Panic if we fail to set up the event log for the next stage. This is a fatal
	 * error because, on the FVP platform, BL2 software assumes that a valid
	 * Event Log buffer exists and will use the same Event Log buffer to append image
	 * measurements.
	 */
	panic();
}
