/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 * Copyright (c) 2022, Linaro.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "./include/imx8m_measured_boot.h"
#include <drivers/measured_boot/event_log/event_log.h>
#include <plat/arm/common/plat_arm.h>

/* Event Log data */
static uint8_t event_log[PLAT_IMX_EVENT_LOG_MAX_SIZE];

/* FVP table with platform specific image IDs, names and PCRs */
static const event_log_metadata_t imx8m_event_log_metadata[] = {
	{ BL31_IMAGE_ID, EVLOG_BL31_STRING, PCR_0 },
	{ BL32_IMAGE_ID, EVLOG_BL32_STRING, PCR_0 },
	{ BL32_EXTRA1_IMAGE_ID, EVLOG_BL32_EXTRA1_STRING, PCR_0 },
	{ BL32_EXTRA2_IMAGE_ID, EVLOG_BL32_EXTRA2_STRING, PCR_0 },
	{ BL33_IMAGE_ID, EVLOG_BL33_STRING, PCR_0 },
	{ EVLOG_INVALID_ID, NULL, (unsigned int)(-1) }	/* Terminator */
};

const event_log_metadata_t *plat_event_log_get_metadata(void)
{
	return imx8m_event_log_metadata;
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	/* Calculate image hash and record data in Event Log */
	int err = event_log_measure_and_record(image_data->image_base,
					       image_data->image_size,
					       image_id);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
		      "Failed to ", "record", image_id, err);
		return err;
	}

	return 0;
}

void bl2_plat_mboot_init(void)
{
	event_log_init(event_log, event_log + sizeof(event_log));
	event_log_write_header();
}

void bl2_plat_mboot_finish(void)
{
	int rc = 0;

	/* Event Log address in Non-Secure memory */
	uintptr_t ns_log_addr;

	/* Event Log filled size */
	size_t event_log_cur_size;

	event_log_cur_size = event_log_get_cur_size(event_log);

	rc = imx8m_set_nt_fw_info(event_log_cur_size, &ns_log_addr);
	if (rc != 0) {
		ERROR("%s(): Unable to update %s_FW_CONFIG\n",
		      __func__, "NT");
		/*
		 * It is a fatal error because on i.MX U-boot assumes that
		 * a valid event log exists and will use it to record the
		 * measurements into the fTPM.
		 */
		panic();
	}

	/* Copy Event Log to Non-secure memory */
	(void)memcpy((void *)ns_log_addr, (const void *)event_log,
		     event_log_cur_size);

	/* Ensure that the Event Log is visible in Non-secure memory */
	flush_dcache_range(ns_log_addr, event_log_cur_size);

	dump_event_log((uint8_t *)event_log, event_log_cur_size);
}
