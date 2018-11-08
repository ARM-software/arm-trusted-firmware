/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <context_mgmt.h>
#include <debug.h>
#include <smccc.h>
#include <smccc_helpers.h>
#include <sprt_svc.h>
#include <utils.h>

#include "spm_private.h"

/*******************************************************************************
 * This function handles all SMCs in the range reserved for SPRT.
 ******************************************************************************/
uint64_t sprt_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			  uint64_t x3, uint64_t x4, void *cookie, void *handle,
			  uint64_t flags)
{
	/* SPRT only supported from the Secure world */
	if (is_caller_non_secure(flags) == SMC_FROM_NON_SECURE) {
		SMC_RET1(handle, SMC_UNK);
	}

	assert(handle == cm_get_context(SECURE));

	/*
	 * Only S-EL0 partitions are supported for now. Make the next ERET into
	 * the partition jump directly to S-EL0 instead of S-EL1.
	 */
	cm_set_elr_spsr_el3(SECURE, read_elr_el1(), read_spsr_el1());

	switch (smc_fid) {
	case SPRT_VERSION:
		SMC_RET1(handle, SPRT_VERSION_COMPILED);

	default:
		break;
	}

	WARN("SPRT: Unsupported call 0x%08x\n", smc_fid);
	SMC_RET1(handle, SPRT_NOT_SUPPORTED);
}
