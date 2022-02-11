/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/rss/rss_measured_boot.h>
#include <plat/arm/common/plat_arm.h>

/* Event Log data */
static uint8_t event_log[PLAT_ARM_EVENT_LOG_MAX_SIZE];

/* FVP table with platform specific image IDs, names and PCRs */
const event_log_metadata_t fvp_event_log_metadata[] = {
	{ FW_CONFIG_ID, EVLOG_FW_CONFIG_STRING, PCR_0 },
	{ TB_FW_CONFIG_ID, EVLOG_TB_FW_CONFIG_STRING, PCR_0 },
	{ BL2_IMAGE_ID, EVLOG_BL2_STRING, PCR_0 },

	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

/* FVP table with platform specific image IDs and metadata. Intentionally not a
 * const struct, some members might set by bootloaders during trusted boot.
 */
struct rss_mboot_metadata fvp_rss_mboot_metadata[] = {
	{
		.id = FW_CONFIG_ID,
		.slot = U(6),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_FW_CONFIG_STRING,
		.lock_measurement = true },
	{
		.id = TB_FW_CONFIG_ID,
		.slot = U(7),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_TB_FW_CONFIG_STRING,
		.lock_measurement = true },
	{
		.id = BL2_IMAGE_ID,
		.slot = U(8),
		.signer_id_size = SIGNER_ID_MIN_SIZE,
		.sw_type = RSS_MBOOT_BL2_STRING,
		.lock_measurement = true },

	{
		.id = RSS_MBOOT_INVALID_ID }
};

void bl1_plat_mboot_init(void)
{
	event_log_init(event_log, event_log + sizeof(event_log));
	event_log_write_header();

	rss_measured_boot_init();
}

void bl1_plat_mboot_finish(void)
{
	size_t event_log_cur_size;

	event_log_cur_size = event_log_get_cur_size(event_log);
	int rc = arm_set_tb_fw_info((uintptr_t)event_log,
				    event_log_cur_size);
	if (rc != 0) {
		/*
		 * It is a fatal error because on FVP platform, BL2 software
		 * assumes that a valid Event Log buffer exist and it will use
		 * same Event Log buffer to append image measurements.
		 */
		panic();
	}
}
