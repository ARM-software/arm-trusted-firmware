/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FWU metadata information as per the specification section 4.1:
 * https://developer.arm.com/documentation/den0118/a/
 *
 */

#ifndef FWU_METADATA_H
#define FWU_METADATA_H

#include <stdint.h>
#include <tools_share/uuid.h>

/* Properties of image in a bank */
struct fwu_image_properties {

	/* UUID of the image in this bank */
	uuid_t img_uuid;

	/* [0]: bit describing the image acceptance status –
	 *      1 means the image is accepted
	 * [31:1]: MBZ
	 */
	uint32_t accepted;

	/* reserved (MBZ) */
	uint32_t reserved;

} __packed;

/* Image entry information */
struct fwu_image_entry {

	/* UUID identifying the image type */
	uuid_t img_type_uuid;

	/* UUID of the storage volume where the image is located */
	uuid_t location_uuid;

	/* Properties of images with img_type_uuid in the different FW banks */
	struct fwu_image_properties img_props[NR_OF_FW_BANKS];

} __packed;

/*
 * FWU metadata filled by the updater and consumed by TF-A for
 * various purposes as below:
 * 1. Get active FW bank.
 * 2. Rollback to previous working FW bank.
 * 3. Get properties of all images present in all banks.
 */
struct fwu_metadata {

	/* Metadata CRC value */
	uint32_t crc_32;

	/* Metadata version */
	uint32_t version;

	/* Bank index with which device boots */
	uint32_t active_index;

	/* Previous bank index with which device booted successfully */
	uint32_t previous_active_index;

	/* Image entry information */
	struct fwu_image_entry img_entry[NR_OF_IMAGES_IN_FW_BANK];

} __packed;

#endif /* FWU_METADATA_H */
