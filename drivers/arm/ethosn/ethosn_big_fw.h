/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

/*
 * Big FW binary structure.
 * Must be kept in sync with the Arm(R) Ethos(TM)-N NPU firmware binary layout.
 */
struct ethosn_big_fw {
	uint32_t fw_magic;
	uint32_t fw_ver_major;
	uint32_t fw_ver_minor;
	uint32_t fw_ver_patch;
	uint32_t arch_min;
	uint32_t arch_max;
	uint32_t offset;
	uint32_t size;
	uint32_t code_offset;
	uint32_t code_size;
	uint32_t ple_offset;
	uint32_t ple_size;
	uint32_t vector_table_offset;
	uint32_t vector_table_size;
	uint32_t unpriv_stack_offset;
	uint32_t unpriv_stack_size;
	uint32_t priv_stack_offset;
	uint32_t priv_stack_size;
} __packed;

bool ethosn_big_fw_verify_header(const struct ethosn_big_fw *big_fw,
				 uint32_t npu_arch_ver);
