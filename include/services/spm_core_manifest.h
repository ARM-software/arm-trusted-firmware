/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_CORE_MANIFEST_H
#define SPM_CORE_MANIFEST_H

#include <stdint.h>

/*******************************************************************************
 * Attribute Section
 ******************************************************************************/

typedef struct spm_core_manifest_sect_attribute {
	/*
	 * FFA version (mandatory).
	 */
	uint32_t major_version;
	uint32_t minor_version;

	/*
	 * Run-Time Execution state (optional):
	 * - 0: AArch64 (default)
	 * - 1: AArch32
	 */
	uint32_t exec_state;

	/*
	 * Address of binary image containing SPM Core (optional).
	 */
	uint64_t load_address;

	/*
	 * Offset from the base of the partition's binary image to the entry
	 * point of the partition (optional).
	 */
	uint64_t entrypoint;

	/*
	 * Size of binary image containing SPM Core in bytes (mandatory).
	 */
	uint32_t binary_size;

	/*
	 * ID of the SPMC (mandatory)
	 */
	uint16_t spmc_id;

} spmc_manifest_attribute_t;

#endif /* SPM_CORE_MANIFEST_H */
