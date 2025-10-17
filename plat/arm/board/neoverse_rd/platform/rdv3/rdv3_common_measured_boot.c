/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/desc_image_load.h>
#include <drivers/measured_boot/rse/rse_measured_boot.h>

extern struct rse_mboot_metadata rdv3_rse_mboot_metadata[];

struct rse_mboot_metadata *plat_rse_mboot_get_metadata(void)
{
	return rdv3_rse_mboot_metadata;
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int err;

	/* Calculate image hash and record data in RSE */
	err = rse_mboot_measure_and_record(rdv3_rse_mboot_metadata,
					   image_data->image_base,
					   image_data->image_size,
					   image_id);
	if (err != 0) {
		ERROR("Measure and record failed for image id %u, err (%i)\n",
		      image_id, err);
	}

	return err;
}

int plat_mboot_measure_key(const void *pk_oid, const void *pk_ptr, size_t pk_len)
{
	return rse_mboot_set_signer_id(rdv3_rse_mboot_metadata, pk_oid,
				       pk_ptr, pk_len);
}
