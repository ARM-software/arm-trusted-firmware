
/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/desc_image_load.h>
#include <drivers/measured_boot/rss/rss_measured_boot.h>

extern struct rss_mboot_metadata tc_rss_mboot_metadata[];

struct rss_mboot_metadata *plat_rss_mboot_get_metadata(void)
{
	return tc_rss_mboot_metadata;
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int err;

	/* Calculate image hash and record data in RSS */
	err = rss_mboot_measure_and_record(image_data->image_base,
					   image_data->image_size,
					   image_id);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
		      "Failed to ", "record in RSS", image_id, err);
	}

	return err;
}
