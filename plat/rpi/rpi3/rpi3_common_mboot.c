/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <tpm2.h>
#include <event_measure.h>
#include <event_print.h>
#include <rpi3_measured_boot.h>

/* RPI3 table with platform specific image IDs, names and PCRs */
extern const event_log_metadata_t rpi3_event_log_metadata[];

#if DISCRETE_TPM
extern struct tpm_chip_data tpm_chip_data;
#endif

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	int rc = 0;
	const event_log_metadata_t *metadata_ptr;
	uint8_t digest_buf[MAX_TPML_BUFFER_SIZE] __unused;
	size_t digest_size __unused;
	tpmt_ha *digest __unused;

	metadata_ptr = mboot_find_event_log_metadata(rpi3_event_log_metadata,
						     image_id);
	if (metadata_ptr == NULL) {
		ERROR("Unable to find metadata for image %u.\n", image_id);
		return -1;
	}

#if !DISCRETE_TPM
	/* Calculate image hash and record data in Event Log */
	rc = event_log_measure_and_record(metadata_ptr->pcr,
					  image_data->image_base,
					  image_data->image_size,
					  metadata_ptr->name,
					  strlen(metadata_ptr->name) + 1U);
	if (rc != 0) {
		ERROR("Image measurement and recording failed (%d).\n", rc);
		return rc;
	}
#else
	/* Calculate image hash and record data in Event Log */
	rc = event_log_measure(image_data->image_base, image_data->image_size,
			       digest_buf, sizeof(digest_buf));
	if (rc != 0) {
		ERROR("Image measurement failed (%d).\n", rc);
		return rc;
	}

	/* Extend measurement to Event Log. */
	rc = event_log_write_pcr_event2(metadata_ptr->pcr, EV_POST_CODE,
					(const tpml_digest_values *)digest_buf,
					(const uint8_t *)metadata_ptr->name,
					strlen(metadata_ptr->name) + 1);
	if (rc != 0) {
		ERROR("Failed to record image measurement to event log (%d).\n",
		      rc);
		return rc;
	}

	/*
	 * TODO: The TPM library currently supports extending only a single digest
	 * at a time.  In practice, we should query the TPM to determine which hash
	 * algorithms it supports, and update the library to allow submitting
	 * multiple digest extensions in one call.
	 */
	digest = ((tpml_digest_values *)digest_buf)->digests;

	rc = tpm_pcr_extend(&tpm_chip_data, PCR_0, digest->algorithm_id,
			    digest->digest, TCG_DIGEST_SIZE);
	if (rc != 0) {
		ERROR("BL2: TPM PCR-0 extend failed\n");
		panic();
	}
#endif /* !DISCRETE_TPM */

	return rc;
}
