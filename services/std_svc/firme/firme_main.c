/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <common/debug.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/smccc.h>
#include <services/firme_svc.h>
#include <smccc_helpers.h>

static inline bool is_base_service_fid(uint32_t fid)
{
	switch (fid) {
	case FIRME_SERVICE_VERSION_FID:
	case FIRME_SERVICE_FEATURES_FID:
		return true;
	default:
		return false;
	}
}

static inline bool is_granule_mgmt_service_fid(uint32_t fid)
{
	switch (fid) {
	case FIRME_GM_GPI_SET_FID:
	case FIRME_GM_GPI_OP_CONTINUE_FID:
	case FIRME_GM_L1_GPT_CREATE_FID:
	case FIRME_GM_L1_GPT_DESTROY_FID:
		return true;
	default:
		return false;
	}
}

static inline bool is_ide_key_mgmt_service_fid(uint32_t fid)
{
	switch (fid) {
	case FIRME_IDE_KEYSET_PROG_FID:
	case FIRME_IDE_KEYSET_GO_FID:
	case FIRME_IDE_KEYSET_STOP_FID:
	case FIRME_IDE_KEYSET_POLL_FID:
		return true;
	default:
		return false;
	}
}

static inline bool is_mecid_service_fid(uint32_t fid)
{
	if (fid == FIRME_MEC_REFRESH_FID) {
		return true;
	} else {
		return false;
	}
}

static inline bool is_attestation_service_fid(uint32_t fid)
{
	switch (fid) {
	case FIRME_ATTEST_PAT_GET_FID:
	case FIRME_ATTEST_RAK_GET_FID:
	case FIRME_ATTEST_RAT_SIGN_FID:
	case FIRME_ATTEST_PAT_EXT_CLAIMS_STAGE_FID:
	case FIRME_ATTEST_PAT_EXT_CLAIMS_CLEAR_FID:
	case FIRME_ATTEST_PAT_EXT_CLAIMS_FINALISE_FID:
		return true;
	default:
		return false;
	}
}

static inline bool is_integrated_device_mgmt_service_fid(uint32_t fid)
{
	switch (fid) {
	case FIRME_IDEV_OP_START_FID:
	case FIRME_IDEV_OP_CONTINUE_FID:
		return true;
	default:
		return false;
	}
}

static inline firme_instance_e get_instance_from_flags(uint64_t flags)
{
	switch (caller_sec_state(flags)) {
	case SMC_FROM_NON_SECURE:
		return FIRME_NONSECURE;
	case SMC_FROM_SECURE:
		return FIRME_SECURE;
	case SMC_FROM_REALM:
		return FIRME_REALM;
	}
	panic();
}

int32_t firme_init(void)
{
	return firme_mecid_service_init();
}

uint64_t firme_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
		       uint64_t x4, void *cookie, void *handle, uint64_t flags)
{
	firme_instance_e instance = get_instance_from_flags(flags);

	/* Determine which FIRME service needs to handle this call */
	if (is_base_service_fid(smc_fid)) {
		return firme_base_service_handler(instance, smc_fid, x1, x2, x3,
						  x4, cookie, handle, flags);
	}

	else if (is_granule_mgmt_service_fid(smc_fid)) {
		return firme_granule_mgmt_service_handler(instance, smc_fid, x1,
							  x2, x3, x4, cookie,
							  handle, flags);
	}

	else if (is_ide_key_mgmt_service_fid(smc_fid)) {
	}

	else if (is_mecid_service_fid(smc_fid)) {
		return firme_mecid_service_handler(instance, smc_fid, x1, x2,
						   x3, x4, cookie, handle,
						   flags);
	}

	else if (is_attestation_service_fid(smc_fid)) {
	}

	else if (is_integrated_device_mgmt_service_fid(smc_fid)) {
	}

	ERROR("FIRME ABI 0x%X is not supported.\n", smc_fid);
	SMC_RET1(handle, FIRME_NOT_SUPPORTED);
}
