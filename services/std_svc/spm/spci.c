/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <smccc.h>
#include <smccc_helpers.h>
#include <spci_svc.h>
#include <utils.h>

#include "spm_private.h"

/*******************************************************************************
 * This function handles all SMCs in the range reserved for SPCI.
 ******************************************************************************/
uint64_t spci_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			  uint64_t x3, uint64_t x4, void *cookie, void *handle,
			  uint64_t flags)
{
	uint32_t spci_fid;

	/* SPCI only supported from the Non-secure world for now */
	if (is_caller_non_secure(flags) == SMC_FROM_SECURE) {
		SMC_RET1(handle, SMC_UNK);
	}

	if ((smc_fid & SPCI_FID_TUN_FLAG) == 0) {

		/* Miscellaneous calls */

		spci_fid = (smc_fid >> SPCI_FID_MISC_SHIFT) & SPCI_FID_MISC_MASK;

		switch (spci_fid) {

		case SPCI_FID_VERSION:
			SMC_RET1(handle, SPCI_VERSION_COMPILED);

		default:
			break;
		}

	} else {

		/* Tunneled calls */

	}

	WARN("SPCI: Unsupported call 0x%08x\n", smc_fid);
	SMC_RET1(handle, SPCI_NOT_SUPPORTED);
}
