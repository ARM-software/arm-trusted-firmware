
/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/desc_image_load.h>
#include <drivers/measured_boot/rse/dice_prot_env.h>

extern struct dpe_metadata tc_dpe_metadata[];

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int err;

	/* Calculate image hash and record it in the DPE service in RSE. */
	err = dpe_measure_and_record(tc_dpe_metadata,
				     image_data->image_base,
				     image_data->image_size,
				     image_id);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
		      "Failed to ", "record in DPE", image_id, err);
	}

	return err;
}

int plat_mboot_measure_key(const void *pk_oid, const void *pk_ptr,
			   size_t pk_len)
{
	return dpe_set_signer_id(tc_dpe_metadata, pk_oid, pk_ptr, pk_len);
}
