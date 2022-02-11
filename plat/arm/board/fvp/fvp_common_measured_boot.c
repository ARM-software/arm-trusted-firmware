/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/desc_image_load.h>
#include <drivers/measured_boot/event_log/event_log.h>
#include <drivers/measured_boot/rss/rss_measured_boot.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

extern event_log_metadata_t fvp_event_log_metadata[];
extern struct rss_mboot_metadata fvp_rss_mboot_metadata[];

const event_log_metadata_t *plat_event_log_get_metadata(void)
{
	return fvp_event_log_metadata;
}

struct rss_mboot_metadata *plat_rss_mboot_get_metadata(void)
{
	return fvp_rss_mboot_metadata;
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int err;
	int rc = 0;

	/* Calculate image hash and record data in Event Log */
	err = event_log_measure_and_record(image_data->image_base,
					   image_data->image_size,
					   image_id);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
		      "Failed to ", "record in event log", image_id, err);
		rc = err;
	}

	/* Calculate image hash and record data in RSS */
	err = rss_mboot_measure_and_record(image_data->image_base,
					   image_data->image_size,
					   image_id);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
		      "Failed to ", "record in RSS", image_id, err);
		rc = (rc == 0) ? err : -1;
	}

	return rc;
}
