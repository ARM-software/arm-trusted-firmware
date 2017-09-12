/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <css_def.h>
#include <stdint.h>
#include <string.h>

#include "sds.h"
#include "sds_private.h"

/*
 * Variables used to track and maintain the state of the memory region reserved
 * for usage by the SDS framework.
 */

/* Pointer to the base of the SDS memory region */
static uintptr_t sds_mem_base;

/* Size of the SDS memory region in bytes */
static size_t sds_mem_size;

/*
 * Perform some non-exhaustive tests to determine whether any of the fields
 * within a Structure Header contain obviously invalid data.
 * Returns SDS_OK on success, SDS_ERR_FAIL on error.
 */
static int sds_struct_is_valid(uintptr_t header)
{
	size_t struct_size = GET_SDS_HEADER_STRUCT_SIZE(header);

	/* Zero is not a valid identifier */
	if (GET_SDS_HEADER_ID(header) == 0)
		return SDS_ERR_FAIL;

	/* Check SDS Schema version */
	if (GET_SDS_HEADER_VERSION(header) == SDS_REGION_SCH_VERSION)
		return SDS_ERR_FAIL;

	/* The SDS Structure sizes have to be multiple of 8 */
	if ((struct_size == 0) || ((struct_size % 8) != 0))
		return SDS_ERR_FAIL;

	if (struct_size > sds_mem_size)
		return SDS_ERR_FAIL;

	return SDS_OK;
}

/*
 * Validate the SDS structure headers.
 * Returns SDS_OK on success, SDS_ERR_FAIL on error.
 */
static int validate_sds_struct_headers(void)
{
	unsigned int i, structure_count;
	uintptr_t header;

	structure_count = GET_SDS_REGION_STRUCTURE_COUNT(sds_mem_base);

	if (structure_count == 0)
		return SDS_ERR_FAIL;

	header = sds_mem_base + SDS_REGION_DESC_SIZE;

	/* Iterate over structure headers and validate each one */
	for (i = 0; i < structure_count; i++) {
		if (sds_struct_is_valid(header) != SDS_OK) {
			WARN("SDS: Invalid structure header detected\n");
			return SDS_ERR_FAIL;
		}
		header += GET_SDS_HEADER_STRUCT_SIZE(header) + SDS_HEADER_SIZE;
	}
	return SDS_OK;
}

/*
 * Get the structure header pointer corresponding to the structure ID.
 * Returns SDS_OK on success, SDS_ERR_STRUCT_NOT_FOUND on error.
 */
static int get_struct_header(uint32_t structure_id, struct_header_t **header)
{
	unsigned int i, structure_count;
	uintptr_t current_header;

	assert(header);

	structure_count = GET_SDS_REGION_STRUCTURE_COUNT(sds_mem_base);
	if (structure_count == 0)
		return SDS_ERR_STRUCT_NOT_FOUND;

	current_header = ((uintptr_t)sds_mem_base) + SDS_REGION_DESC_SIZE;

	/* Iterate over structure headers to find one with a matching ID */
	for (i = 0; i < structure_count; i++) {
		if (GET_SDS_HEADER_ID(current_header) == structure_id) {
			*header = (struct_header_t *)current_header;
			return SDS_OK;
		}
		current_header += GET_SDS_HEADER_STRUCT_SIZE(current_header) +
						SDS_HEADER_SIZE;
	}

	*header = NULL;
	return SDS_ERR_STRUCT_NOT_FOUND;
}

/*
 * Check if a structure header corresponding to the structure ID exists.
 * Returns SDS_OK if structure header exists else SDS_ERR_STRUCT_NOT_FOUND
 * if not found.
 */
int sds_struct_exists(unsigned int structure_id)
{
	struct_header_t *header = NULL;
	int ret;

	ret = get_struct_header(structure_id, &header);
	if (ret == SDS_OK) {
		assert(header);
	}

	return ret;
}

/*
 * Read from field in the structure corresponding to `structure_id`.
 * `fld_off` is the offset to the field in the structure and `mode`
 * indicates whether cache maintenance need to performed prior to the read.
 * The `data` is the pointer to store the read data of size specified by `size`.
 * Returns SDS_OK on success or corresponding error codes on failure.
 */
int sds_struct_read(uint32_t structure_id, unsigned int fld_off,
		void *data, size_t size, sds_access_mode_t mode)
{
	int status;
	uintptr_t field_base;
	struct_header_t *header = NULL;

	if (!data)
		return SDS_ERR_INVALID_PARAMS;

	/* Check if a structure with this ID exists */
	status = get_struct_header(structure_id, &header);
	if (status != SDS_OK)
		return status;

	assert(header);

	if (mode == SDS_ACCESS_MODE_CACHED)
		inv_dcache_range((uintptr_t)header, SDS_HEADER_SIZE + size);

	if (!IS_SDS_HEADER_VALID(header)) {
		WARN("SDS: Reading from un-finalized structure 0x%x\n",
				structure_id);
		return SDS_ERR_STRUCT_NOT_FINALIZED;
	}

	if ((fld_off + size) > GET_SDS_HEADER_STRUCT_SIZE(header))
		return SDS_ERR_FAIL;

	field_base = (uintptr_t)header + SDS_HEADER_SIZE + fld_off;
	if (check_uptr_overflow(field_base, size - 1))
		return SDS_ERR_FAIL;

	/* Copy the required field in the struct */
	memcpy(data, (void *)field_base, size);

	return SDS_OK;
}

/*
 * Write to the field in the structure corresponding to `structure_id`.
 * `fld_off` is the offset to the field in the structure and `mode`
 * indicates whether cache maintenance need to performed for the write.
 * The `data` is the pointer to data of size specified by `size`.
 * Returns SDS_OK on success or corresponding error codes on failure.
 */
int sds_struct_write(uint32_t structure_id, unsigned int fld_off,
		void *data, size_t size, sds_access_mode_t mode)
{
	int status;
	uintptr_t field_base;
	struct_header_t *header = NULL;

	if (!data)
		return SDS_ERR_INVALID_PARAMS;

	/* Check if a structure with this ID exists */
	status = get_struct_header(structure_id, &header);
	if (status != SDS_OK)
		return status;

	assert(header);

	if (mode == SDS_ACCESS_MODE_CACHED)
		inv_dcache_range((uintptr_t)header, SDS_HEADER_SIZE + size);

	if (!IS_SDS_HEADER_VALID(header)) {
		WARN("SDS: Writing to un-finalized structure 0x%x\n",
				structure_id);
		return SDS_ERR_STRUCT_NOT_FINALIZED;
	}

	if ((fld_off + size) > GET_SDS_HEADER_STRUCT_SIZE(header))
		return SDS_ERR_FAIL;

	field_base = (uintptr_t)header + SDS_HEADER_SIZE + fld_off;
	if (check_uptr_overflow(field_base, size - 1))
		return SDS_ERR_FAIL;

	/* Copy the required field in the struct */
	memcpy((void *)field_base, data, size);

	if (mode == SDS_ACCESS_MODE_CACHED)
		flush_dcache_range((uintptr_t)field_base, size);

	return SDS_OK;
}

/*
 * Initialize the SDS driver. Also verifies the SDS version and sanity of
 * the SDS structure headers.
 * Returns SDS_OK on success, SDS_ERR_FAIL on error.
 */
int sds_init(void)
{
	sds_mem_base = (uintptr_t)PLAT_ARM_SDS_MEM_BASE;

	if (!IS_SDS_REGION_VALID(sds_mem_base)) {
		WARN("SDS: No valid SDS Memory Region found\n");
		return SDS_ERR_FAIL;
	}

	if (GET_SDS_REGION_SCHEMA_VERSION(sds_mem_base)
				!= SDS_REGION_SCH_VERSION) {
		WARN("SDS: Unsupported SDS schema version\n");
		return SDS_ERR_FAIL;
	}

	sds_mem_size = GET_SDS_REGION_SIZE(sds_mem_base);
	if (sds_mem_size > PLAT_ARM_SDS_MEM_SIZE_MAX) {
		WARN("SDS: SDS Memory Region exceeds size limit\n");
		return SDS_ERR_FAIL;
	}

	INFO("SDS: Detected SDS Memory Region (%zu bytes)\n", sds_mem_size);

	if (validate_sds_struct_headers() != SDS_OK)
		return SDS_ERR_FAIL;

	return SDS_OK;
}
