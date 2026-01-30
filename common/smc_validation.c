/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/smc_validation_framework.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <smccc_helpers.h>

#pragma weak plat_is_valid_ns_address_range

bool plat_is_valid_ns_address_range(uintptr_t base, size_t size)
{
	return true;
}

int smc_get_arg(void *handle, unsigned int arg_idx, u_register_t *val)
{
#ifdef __aarch64__
	static const uint16_t x_offsets[] = {
		CTX_GPREG_X0,  CTX_GPREG_X1,  CTX_GPREG_X2,  CTX_GPREG_X3,
		CTX_GPREG_X4,  CTX_GPREG_X5,  CTX_GPREG_X6,  CTX_GPREG_X7,
		CTX_GPREG_X8,  CTX_GPREG_X9,  CTX_GPREG_X10, CTX_GPREG_X11,
		CTX_GPREG_X12, CTX_GPREG_X13, CTX_GPREG_X14, CTX_GPREG_X15,
		CTX_GPREG_X16, CTX_GPREG_X17
	};
#else
	static const uint16_t x_offsets[] = { CTX_GPREG_R0, CTX_GPREG_R1,
					      CTX_GPREG_R2, CTX_GPREG_R3 };
#endif

	/* SMCCC registers are x1 to x17 (AArch64) or r1 to r7 (AArch32). */
	if (arg_idx >= ARRAY_SIZE(x_offsets)) {
		return SMC_INVALID_PARAM;
	}

#ifdef __aarch64__
	*val = (u_register_t)read_ctx_reg(get_gpregs_ctx(handle),
					  x_offsets[arg_idx]);
#else
	*val = (u_register_t)read_ctx_reg(get_regs_ctx(handle),
					  x_offsets[arg_idx]);
#endif

	return SMC_OK;
}

int smc_validate_mem_range(uintptr_t base, size_t size)
{
	/* Reject if base + size overflows. */
	if ((size > 0U) && check_uptr_overflow(base, (uintptr_t)size)) {
		return SMC_INVALID_PARAM;
	}

	/* Reject if the range is not in Non-Secure memory. */
	if (!plat_is_valid_ns_address_range(base, size)) {
		return SMC_INVALID_PARAM;
	}

	return SMC_OK;
}

int smc_get_mem_range(void *handle, unsigned int base_reg,
		      unsigned int size_reg, uintptr_t *base, size_t *size)
{
	u_register_t base_reg_val;
	u_register_t size_reg_val;

	if (smc_get_arg(handle, base_reg, &base_reg_val) != SMC_OK) {
		return SMC_INVALID_PARAM;
	}

	if (smc_get_arg(handle, size_reg, &size_reg_val) != SMC_OK) {
		return SMC_INVALID_PARAM;
	}

	uintptr_t base_local = (uintptr_t)base_reg_val;
	size_t size_local = (size_t)size_reg_val;

	if (smc_validate_mem_range(base_local, size_local) != SMC_OK) {
		return SMC_INVALID_PARAM;
	}

	*base = base_local;
	*size = size_local;

	return SMC_OK;
}
