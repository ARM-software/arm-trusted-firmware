/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMC_VALIDATION_FRAMEWORK_H
#define SMC_VALIDATION_FRAMEWORK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/smccc.h>
#include <lib/utils_def.h>

/**
 * Platform Hook: Verify range belongs to Non-Secure world (GPT/DRAM).
 */
bool plat_is_valid_ns_address_range(uintptr_t base, size_t size);

/**
 * smc_get_arg() - Fetch registers x1 to x17 from the cpu_context.
 * @handle: CPU context handle.
 * @arg_idx: Register index (e.g., 1 for x1).
 * @val: Pointer to store the register value.
 *
 * Returns SMC_OK on success, or SMC_INVALID_PARAM if arg_idx is out of bounds.
 */
int smc_get_arg(void *handle, unsigned int arg_idx, u_register_t *val);

/**
 * smc_validate_mem_range() - Tuple-based validation with Integer Overflow
 * protection.
 * @base: The starting physical address (Shadow-Copy).
 * @size: The size of the memory range (Shadow-Copy).
 *
 * Returns SMC_OK on success, or SMC_INVALID_PARAM on failure.
 */
int smc_validate_mem_range(uintptr_t base, size_t size);

/**
 * smc_get_mem_range() - Unmarshal and validate a memory range tuple.
 * @handle: CPU context handle.
 * @base_reg: Register index for base address (e.g., 1 for x1).
 * @size_reg: Register index for range size.
 * @base: Pointer to store the unmarshaled base address.
 * @size: Pointer to store the unmarshaled size.
 *
 * Returns SMC_OK on success, or SMC_INVALID_PARAM on failure.
 */
int smc_get_mem_range(void *handle, unsigned int base_reg,
		      unsigned int size_reg, uintptr_t *base, size_t *size);

/**
 * Example Usage in a Validated Handler:
 *
 * static uintptr_t sip_handler(uint32_t smc_fid, u_register_t x1 ... void
 * *handle) {
 *	uintptr_t buf;
 *	size_t len;
 *
 *	u_register_t cmd_reg;
 *
 *	if (smc_get_arg(handle, 1, &cmd_reg) != SMC_OK) {
 *		SMC_RET1(handle, SMC_INVALID_PARAM);
 *	}
 *
 *	uint32_t cmd = (uint32_t)cmd_reg;
 *
 *	if (smc_get_mem_range(handle, 2, 3, &buf, &len) != SMC_OK) {
 *		SMC_RET1(handle, SMC_INVALID_PARAM);
 *	}
 *
 *	// 'buf' is now 100% verified to be in Non-Secure RAM and overflow-safe.
 *	return plat_common_handler(cmd, buf, len);
 * }
 */

#endif /* SMC_VALIDATION_FRAMEWORK_H */
