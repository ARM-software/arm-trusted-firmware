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

/*
 * Ensure that the NR_OF_FW_BANKS selected by the platform is not
 * zero and not greater than the maximum number of banks allowed
 * by the specification.
 */
CASSERT((NR_OF_FW_BANKS > 0) && (NR_OF_FW_BANKS <= NR_OF_MAX_FW_BANKS),
	assert_fwu_num_banks_invalid_value);

#define FWU_METADATA_VERSION		2U
#define FWU_FW_STORE_DESC_OFFSET	0x20U

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
 * return -EINVAL on error, otherwise 0
 ******************************************************************************/
static int fwu_metadata_sanity_check(void)
{
	if (metadata.version != FWU_METADATA_VERSION) {
		WARN("Incorrect FWU Metadata version of %u\n",
		     metadata.version);
		return -EINVAL;
	}

	if (metadata.active_index >= NR_OF_FW_BANKS) {
		WARN("Active Index value(%u) greater than the configured value(%d)",
		     metadata.active_index, NR_OF_FW_BANKS);
		return -EINVAL;
	}

	if (metadata.previous_active_index >= NR_OF_FW_BANKS) {
		WARN("Previous Active Index value(%u) greater than the configured value(%d)",
		     metadata.previous_active_index, NR_OF_FW_BANKS);
		return -EINVAL;
	}

#if PSA_FWU_METADATA_FW_STORE_DESC
	if (metadata.fw_desc.num_banks != NR_OF_FW_BANKS) {
		WARN("Number of Banks(%u) in FWU Metadata different from the configured value(%d)",
		     metadata.fw_desc.num_banks, NR_OF_FW_BANKS);
		return -EINVAL;
	}

	if (metadata.fw_desc.num_images != NR_OF_IMAGES_IN_FW_BANK) {
		WARN("Number of Images(%u) in FWU Metadata different from the configured value(%d)",
		     metadata.fw_desc.num_images, NR_OF_IMAGES_IN_FW_BANK);
		return -EINVAL;
	}

	if (metadata.desc_offset != FWU_FW_STORE_DESC_OFFSET) {
		WARN("Descriptor Offset(0x%x) in the FWU Metadata not equal to 0x20\n",
		     metadata.desc_offset);
		return -EINVAL;
	}
#else
	if (metadata.desc_offset != 0U) {
		WARN("Descriptor offset has non zero value of 0x%x\n",
		     metadata.desc_offset);
		return -EINVAL;
	}
#endif

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
 * Check for an alternate bank for the platform to boot from. This function will
 * mostly be called whenever the count of the number of times a platform boots
 * in the Trial State exceeds a pre-set limit.
 * The function first checks if the platform can boot from the previously active
 * bank. If not, it tries to find another bank in the accepted state.
 * And finally, if both the checks fail, as a last resort, it tries to find
 * a valid bank.
 *
 * Returns the index of a bank to boot, else returns invalid index
 * INVALID_BOOT_IDX.
 ******************************************************************************/
uint32_t fwu_get_alternate_boot_bank(void)
{
	uint32_t i;

	/* First check if the previously active bank can be used */
	if (metadata.bank_state[metadata.previous_active_index] ==
	    FWU_BANK_STATE_ACCEPTED) {
		return metadata.previous_active_index;
	}

	/* Now check for any other bank in the accepted state */
	for (i = 0U; i < NR_OF_FW_BANKS; i++) {
		if (i == metadata.active_index ||
		    i == metadata.previous_active_index) {
			continue;
		}

		if (metadata.bank_state[i] == FWU_BANK_STATE_ACCEPTED) {
			return i;
		}
	}

	/*
	 * No accepted bank found. Now try booting from a valid bank.
	 * Give priority to the previous active bank.
	 */
	if (metadata.bank_state[metadata.previous_active_index] ==
	    FWU_BANK_STATE_VALID) {
		return metadata.previous_active_index;
	}

	for (i = 0U; i < NR_OF_FW_BANKS; i++) {
		if (i == metadata.active_index ||
		    i == metadata.previous_active_index) {
			continue;
		}

		if (metadata.bank_state[i] == FWU_BANK_STATE_VALID) {
			return i;
		}
	}

	return INVALID_BOOT_IDX;
}

/*******************************************************************************
 * The platform can be in one of Valid, Invalid or Accepted states.
 *
 * Invalid - One or more images in the bank are corrupted, or partially
 *           overwritten. The bank is not to be used for booting.
 *
 * Valid - All images of the bank are valid but at least one image has not
 *         been accepted. This implies that the platform is in Trial State.
 *
 * Accepted - All images of the bank are valid and accepted.
 *
 * Returns the state of the current active bank
 ******************************************************************************/
uint32_t fwu_get_active_bank_state(void)
{
	assert(is_metadata_initialized);

	return metadata.bank_state[metadata.active_index];
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
