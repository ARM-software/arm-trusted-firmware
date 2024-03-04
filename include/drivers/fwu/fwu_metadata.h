/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FWU metadata information as per the specification section 4.1:
 * https://developer.arm.com/documentation/den0118/latest/
 *
 */

#ifndef FWU_METADATA_H
#define FWU_METADATA_H

#include <stdint.h>
#include <tools_share/uuid.h>

#define NR_OF_MAX_FW_BANKS	4

/* Properties of image in a bank */
struct fwu_image_bank_info {

	/* GUID of the image in this bank */
	struct efi_guid img_guid;

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

	/* GUID identifying the image type */
	struct efi_guid img_type_guid;

	/* GUID of the storage volume where the image is located */
	struct efi_guid location_guid;

	/* Properties of images with img_type_guid in the different FW banks */
	struct fwu_image_bank_info img_bank_info[NR_OF_FW_BANKS];

} __packed;

/* Firmware Image descriptor */
struct fwu_fw_store_descriptor {

	/* Number of Banks */
	uint8_t num_banks;

	/* Reserved */
	uint8_t reserved;

	/* Number of images per bank */
	uint16_t num_images;

	/* Size of image_entry(all banks) in bytes */
	uint16_t img_entry_size;

	/* Size of image bank info structure in bytes */
	uint16_t bank_info_entry_size;

	/* Array of fwu_image_entry structs */
	struct fwu_image_entry img_entry[NR_OF_IMAGES_IN_FW_BANK];

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

	/* Size of the entire metadata in bytes */
	uint32_t metadata_size;

	/* Offset of the image descriptor structure */
	uint16_t desc_offset;

	/* Reserved */
	uint16_t reserved1;

	/* Bank state */
	uint8_t bank_state[NR_OF_MAX_FW_BANKS];

	/* Reserved */
	uint32_t reserved2;

#if PSA_FWU_METADATA_FW_STORE_DESC
	/* Image entry information */
	struct fwu_fw_store_descriptor fw_desc;
#endif

} __packed;

#endif /* FWU_METADATA_H */
