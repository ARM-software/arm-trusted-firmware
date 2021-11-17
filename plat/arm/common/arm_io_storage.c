/*
 * Copyright (c) 2015-2021, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/fwu/fwu_metadata.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_storage.h>
#include <drivers/partition/partition.h>
#include <lib/utils.h>

#include <plat/arm/common/arm_fconf_getter.h>
#include <plat/arm/common/arm_fconf_io_storage.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

/* IO devices */
static const io_dev_connector_t *fip_dev_con;
uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
uintptr_t memmap_dev_handle;

#if ARM_GPT_SUPPORT
/* fip partition names */
static const char * const fip_part_names[] = {"FIP_A", "FIP_B"};
CASSERT(sizeof(fip_part_names)/sizeof(char *) == NR_OF_FW_BANKS,
	assert_fip_partition_names_missing);
#endif /* ARM_GPT_SUPPORT */

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_arm_io_setup
#pragma weak plat_arm_get_alt_image_source

int open_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

int open_memmap(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(memmap_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(memmap_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using Memmap\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

int arm_io_setup(void)
{
	int io_result;

	io_result = register_io_dev_fip(&fip_dev_con);
	if (io_result < 0) {
		return io_result;
	}

	io_result = register_io_dev_memmap(&memmap_dev_con);
	if (io_result < 0) {
		return io_result;
	}

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	if (io_result < 0) {
		return io_result;
	}

	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&memmap_dev_handle);

	return io_result;
}

void plat_arm_io_setup(void)
{
	int err;

	err = arm_io_setup();
	if (err < 0) {
		panic();
	}
}

int plat_arm_get_alt_image_source(
	unsigned int image_id __unused,
	uintptr_t *dev_handle __unused,
	uintptr_t *image_spec __unused)
{
	/* By default do not try an alternative */
	return -ENOENT;
}

/* Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	policy = FCONF_GET_PROPERTY(arm, io_policies, image_id);
	result = policy->check(policy->image_spec);
	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	} else {
		VERBOSE("Trying alternative IO\n");
		result = plat_arm_get_alt_image_source(image_id, dev_handle,
						       image_spec);
	}

	return result;
}

/*
 * See if a Firmware Image Package is available,
 * by checking if TOC is valid or not.
 */
bool arm_io_is_toc_valid(void)
{
	return (io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID) == 0);
}

#if ARM_GPT_SUPPORT
/******************************************************************************
 * Retrieve partition entry details such as offset and length, and set these
 * details in the I/O policy of the requested image.
 *
 * @image_id: image id whose I/O policy to be updated
 *
 * @part_name: partition name whose details to be retrieved
 *
 * Returns 0 on success, error otherwise
 * Alongside, returns device handle and image specification of requested
 * image.
 ******************************************************************************/
int arm_set_image_source(unsigned int image_id, const char *part_name,
			 uintptr_t *dev_handle, uintptr_t *image_spec)
{
	const partition_entry_t *entry = get_partition_entry(part_name);

	if (entry == NULL) {
		ERROR("Unable to find the %s partition\n", part_name);
		return -ENOENT;
	}

	struct plat_io_policy *policy = FCONF_GET_PROPERTY(arm,
							   io_policies,
							   image_id);

	assert(policy != NULL);
	assert(policy->image_spec != 0UL);

	io_block_spec_t *spec = (io_block_spec_t *)policy->image_spec;
	/* set offset and length of the image */
	spec->offset = PLAT_ARM_FLASH_IMAGE_BASE + entry->start;
	spec->length = entry->length;

	*dev_handle = *(policy->dev_handle);
	*image_spec = policy->image_spec;

	return 0;
}

/*******************************************************************************
 * Set the source offset and length of the FIP image in its I/O policy.
 *
 * @active_fw_bank_idx: active firmware bank index gathered from FWU metadata.
 ******************************************************************************/
void arm_set_fip_addr(uint32_t active_fw_bank_idx)
{
	uintptr_t dev_handle __unused;
	uintptr_t image_spec __unused;

	assert(active_fw_bank_idx < NR_OF_FW_BANKS);

	INFO("Booting with partition %s\n", fip_part_names[active_fw_bank_idx]);

	int result = arm_set_image_source(FIP_IMAGE_ID,
					  fip_part_names[active_fw_bank_idx],
					  &dev_handle,
					  &image_spec);
	if (result != 0) {
		panic();
	}
}
#endif /* ARM_GPT_SUPPORT */

#if PSA_FWU_SUPPORT
/*******************************************************************************
 * Read the FIP partition of the GPT image corresponding to the active firmware
 * bank to get its offset and length, and update these details in the I/O policy
 * of the FIP image.
 ******************************************************************************/
void plat_fwu_set_images_source(const struct fwu_metadata *metadata)
{
	arm_set_fip_addr(metadata->active_index);
}

/*******************************************************************************
 * Read the requested FWU metadata partition of the GPT image to get its offset
 * and length, and update these details in the I/O policy of the requested FWU
 * metadata image.
 ******************************************************************************/
int plat_fwu_set_metadata_image_source(unsigned int image_id,
				       uintptr_t *dev_handle,
				       uintptr_t *image_spec)
{
	int result = -1;

	if (image_id == FWU_METADATA_IMAGE_ID) {
		result = arm_set_image_source(FWU_METADATA_IMAGE_ID,
					      "FWU-Metadata",
					      dev_handle,
					      image_spec);
	} else if (image_id == BKUP_FWU_METADATA_IMAGE_ID) {
		result = arm_set_image_source(BKUP_FWU_METADATA_IMAGE_ID,
					      "Bkup-FWU-Metadata",
					      dev_handle,
					      image_spec);
	}

	return result;
}
#endif /* PSA_FWU_SUPPORT */
