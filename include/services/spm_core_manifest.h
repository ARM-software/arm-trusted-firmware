/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMC_MANIFEST_H
#define SPMC_MANIFEST_H

#include <stdint.h>

/*******************************************************************************
 * Attribute Section
 ******************************************************************************/

typedef struct spm_core_manifest_sect_attribute {
	/*
	 * SPCI version (mandatory).
	 */
	uint32_t major_version;
	uint32_t minor_version;

	/*
	 * Run-Time Exception Level (mandatory):
	 * - 1: SEL1
	 * - 2: SEL2
	 */
	uint32_t runtime_el;

	/*
	 * Run-Time Execution state (optional):
	 * - 0: AArch64 (default)
	 * - 1: AArch32
	 */
	uint32_t exec_state;

	/*
	 * Address of binary image containing SPM core in bytes (optional).
	 */
	uint64_t load_address;

	/*
	 * Offset from the base of the partition's binary image to the entry
	 * point of the partition.
	 */
	uint64_t entrypoint;

	/*
	 * Size of binary image containing SPM core in bytes (mandatory).
	 */
	uint32_t binary_size;

} spmc_manifest_sect_attribute_t;

#endif /* SPMC_MANIFEST_H */
