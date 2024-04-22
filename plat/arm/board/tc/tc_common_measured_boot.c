/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/desc_image_load.h>
#include <drivers/measured_boot/rse/rse_measured_boot.h>

extern struct rse_mboot_metadata tc_rse_mboot_metadata[];

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int err;

	/* Calculate image hash and record data in RSE */
	err = rse_mboot_measure_and_record(tc_rse_mboot_metadata,
					   image_data->image_base,
					   image_data->image_size,
					   image_id);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
		      "Failed to ", "record in RSE", image_id, err);
	}

	return err;
}

int plat_mboot_measure_key(void *pk_oid, void *pk_ptr, unsigned int pk_len)
{
	return rse_mboot_set_signer_id(tc_rse_mboot_metadata, pk_oid, pk_ptr,
				       pk_len);
}
