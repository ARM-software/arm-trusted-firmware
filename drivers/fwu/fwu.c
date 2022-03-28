/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/tf_crc32.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/fwu/fwu.h>
#include <drivers/fwu/fwu_metadata.h>
#include <drivers/io/io_storage.h>

#include <plat/common/platform.h>

/*
 * Assert that crc_32 is the first member of fwu_metadata structure.
 * It avoids accessing data outside of the metadata structure during
 * CRC32 computation if the crc_32 field gets moved due the structure
 * member(s) addition in the future.
 */
CASSERT((offsetof(struct fwu_metadata, crc_32) == 0),
	crc_32_must_be_first_member_of_structure);

static struct fwu_metadata metadata;
static bool is_metadata_initialized __unused;

/*******************************************************************************
 * Compute CRC32 of the FWU metadata, and check it against the CRC32 value
 * present in the FWU metadata.
 *
 * return -1 on error, otherwise 0
 ******************************************************************************/
static int fwu_metadata_crc_check(void)
{
	unsigned char *data = (unsigned char *)&metadata;

	uint32_t calc_crc = tf_crc32(0U, data + sizeof(metadata.crc_32),
				     (sizeof(metadata) -
				      sizeof(metadata.crc_32)));

	if (metadata.crc_32 != calc_crc) {
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * Check the sanity of FWU metadata.
 *
 * return -1 on error, otherwise 0
 ******************************************************************************/
static int fwu_metadata_sanity_check(void)
{
	/* ToDo: add more conditions for sanity check */
	if ((metadata.active_index >= NR_OF_FW_BANKS) ||
	    (metadata.previous_active_index >= NR_OF_FW_BANKS)) {
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * Verify and load specified FWU metadata image to local FWU metadata structure.
 *
 * @image_id: FWU metadata image id (either FWU_METADATA_IMAGE_ID or
 *				     BKUP_FWU_METADATA_IMAGE_ID)
 *
 * return a negative value on error, otherwise 0
 ******************************************************************************/
static int fwu_metadata_load(unsigned int image_id)
{
	int result;
	uintptr_t dev_handle, image_handle, image_spec;
	size_t bytes_read;

	assert((image_id == FWU_METADATA_IMAGE_ID) ||
	       (image_id == BKUP_FWU_METADATA_IMAGE_ID));

	result = plat_fwu_set_metadata_image_source(image_id,
						    &dev_handle,
						    &image_spec);
	if (result != 0) {
		WARN("Failed to set reference to image id=%u (%i)\n",
		     image_id, result);
		return result;
	}

	result = io_open(dev_handle, image_spec, &image_handle);
	if (result != 0) {
		WARN("Failed to load image id id=%u (%i)\n",
		     image_id, result);
		return result;
	}

	result = io_read(image_handle, (uintptr_t)&metadata,
			 sizeof(struct fwu_metadata), &bytes_read);

	if (result != 0) {
		WARN("Failed to read image id=%u (%i)\n", image_id, result);
		goto exit;
	}

	if (sizeof(struct fwu_metadata) != bytes_read) {
		/* return -1 in case of partial/no read */
		result = -1;
		WARN("Read bytes (%zu) instead of expected (%zu) bytes\n",
		     bytes_read, sizeof(struct fwu_metadata));
		goto exit;
	}

	/* sanity check on loaded parameters */
	result = fwu_metadata_sanity_check();
	if (result != 0) {
		WARN("Sanity %s\n", "check failed on FWU metadata");
		goto exit;
	}

	/* CRC check on loaded parameters */
	result = fwu_metadata_crc_check();
	if (result != 0) {
		WARN("CRC %s\n", "check failed on FWU metadata");
	}

exit:
	(void)io_close(image_handle);

	return result;
}

/*******************************************************************************
 * The system runs in the trial run state if any of the images in the active
 * firmware bank has not been accepted yet.
 *
 * Returns true if the system is running in the trial state.
 ******************************************************************************/
bool fwu_is_trial_run_state(void)
{
	bool trial_run = false;

	assert(is_metadata_initialized);

	for (unsigned int i = 0U; i < NR_OF_IMAGES_IN_FW_BANK; i++) {
		struct fwu_image_entry *entry = &metadata.img_entry[i];
		struct fwu_image_properties *img_props =
			&entry->img_props[metadata.active_index];
		if (img_props->accepted == 0) {
			trial_run = true;
			break;
		}
	}

	return trial_run;
}

const struct fwu_metadata *fwu_get_metadata(void)
{
	assert(is_metadata_initialized);

	return &metadata;
}

/*******************************************************************************
 * Load verified copy of FWU metadata image kept in the platform NV storage
 * into local FWU metadata structure.
 * Also, update platform I/O policies with the offset address and length of
 * firmware-updated images kept in the platform NV storage.
 ******************************************************************************/
void fwu_init(void)
{
	/* Load FWU metadata which will be used to load the images in the
	 * active bank as per PSA FWU specification
	 */
	int result = fwu_metadata_load(FWU_METADATA_IMAGE_ID);

	if (result != 0) {
		WARN("loading of FWU-Metadata failed, "
		     "using Bkup-FWU-Metadata\n");

		result = fwu_metadata_load(BKUP_FWU_METADATA_IMAGE_ID);
		if (result != 0) {
			ERROR("loading of Bkup-FWU-Metadata failed\n");
			panic();
		}
	}

	is_metadata_initialized = true;

	plat_fwu_set_images_source(&metadata);
}
