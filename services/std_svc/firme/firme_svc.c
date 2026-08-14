/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include "firme_private.h"

#include <arch.h>
#include <arch_features.h>
#include <common/debug.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/smccc.h>
#include <services/firme/firme_attestation.h>
#include <services/firme/firme_granule_mgmt.h>
#include <services/firme/firme_ide_km.h>
#include <services/firme/firme_idev.h>
#include <services/firme/firme_mecid.h>
#include <services/firme_svc.h>
#include <smccc_helpers.h>

static const struct firme_service *firme_services[FIRME_SERVICE_ID_MAX] = {
	[FIRME_BASE_ID] = &firme_base_service,
	[FIRME_GRANULE_MGMT_ID] = &firme_granule_mgmt_service,
	[FIRME_MECID_MGMT_ID] = &firme_mecid_service,
#if FIRME_SUPPORT_IDE_KM
	[FIRME_IDE_KEY_MGMT_ID] = &firme_ide_km_service,
#endif
};

#define FIRME_SERVICE_MASK_ALL	((uint16_t)(BIT(FIRME_SERVICE_ID_MAX) - 1U))

#pragma weak plat_firme_get_supported_svcs
int plat_firme_get_supported_svcs(uint16_t *svc_mask)
{
	if (svc_mask == NULL) {
		return -EINVAL;
	}

	*svc_mask = FIRME_SERVICE_MASK_ALL;
	return 0;
}

static inline firme_service_id_e get_service_id_from_fid(uint32_t fid)
{
	switch (fid) {
	case FIRME_SERVICE_VERSION_FID:
	case FIRME_SERVICE_FEATURES_FID:
		return FIRME_BASE_ID;
#if ENABLE_FEAT_RME
	case FIRME_GM_GPI_SET_FID:
	case FIRME_GM_GPI_OP_CONTINUE_FID:
	case FIRME_GM_L1_GPT_CREATE_FID:
	case FIRME_GM_L1_GPT_DESTROY_FID:
		return FIRME_GRANULE_MGMT_ID;
#endif
	case FIRME_IDE_KEYSET_PROG_FID:
	case FIRME_IDE_KEYSET_GO_FID:
	case FIRME_IDE_KEYSET_STOP_FID:
	case FIRME_IDE_KEYSET_POLL_FID:
		return FIRME_IDE_KEY_MGMT_ID;
	case FIRME_MEC_REFRESH_FID:
		return FIRME_MECID_MGMT_ID;
	case FIRME_ATTEST_PAT_GET_FID:
	case FIRME_ATTEST_RAK_GET_FID:
	case FIRME_ATTEST_RAT_SIGN_FID:
	case FIRME_ATTEST_PAT_EXT_CLAIMS_STAGE_FID:
	case FIRME_ATTEST_PAT_EXT_CLAIMS_CLEAR_FID:
	case FIRME_ATTEST_PAT_EXT_CLAIMS_FINALISE_FID:
		return FIRME_ATTESTATION_ID;
	case FIRME_IDEV_OP_START_FID:
	case FIRME_IDEV_OP_CONTINUE_FID:
		return FIRME_INTEGRATED_DEVICE_MGMT_ID;
	default:
		return FIRME_SERVICE_ID_MAX;
	}
}

static inline firme_instance_e get_instance_from_flags(uint64_t flags)
{
	switch (caller_sec_state(flags)) {
	case SMC_FROM_REALM:
		return FIRME_REALM;
	case SMC_FROM_SECURE:
		return FIRME_SECURE;
	default:
		return FIRME_NONSECURE;
	}
}

/*
 * This helper converts generic error codes (from platform hooks), to FIRME
 * error status code.
 */
int firme_errno_from_generic_errno(int errno)
{
	int rc;

	switch (errno) {
	case 0:
		rc = FIRME_SUCCESS;
		break;
	case -ENOTSUP:
		rc = FIRME_NOT_SUPPORTED;
		break;
	case -EINVAL:
		rc = FIRME_INVALID_PARAMETERS;
		break;
	case -EBUSY:
		rc = FIRME_BUSY;
		break;
	case -ECANCELED:
		rc = FIRME_ABORTED;
		break;
	case -EACCES:
		rc = FIRME_DENIED;
		break;
	case -EINPROGRESS:
		rc = FIRME_OP_CONFLICT;
		break;
	case -EAGAIN:
		rc = FIRME_INCOMPLETE;
		break;
	default:
		assert(0);
		rc = FIRME_NOT_SUPPORTED;
	}

	return rc;
}

int32_t firme_init(void)
{
	int32_t rc;
	uint16_t svc_mask = FIRME_SERVICE_MASK_ALL;
	unsigned int i;

	rc = plat_firme_get_supported_svcs(&svc_mask);
	if (rc != 0U) {
		ERROR("Failed to retrieve supported services!");
		return -1;
	}

	if (!((svc_mask & BIT(FIRME_BASE_ID)) &&
		(svc_mask & ~BIT(FIRME_BASE_ID)))) {
		ERROR("Invalid FIRME sevice configuration!");
		return -1;
	}

	for (i = 0U; i < FIRME_SERVICE_ID_MAX; i++) {
		const struct firme_service *service = firme_services[i];

		if (service == NULL) {
			continue;
		}

		if ((svc_mask & BIT(i)) == 0U) {
			VERBOSE("Service %u disabled by platform.\n",
				service->id);
			firme_services[i] = NULL;
			continue;
		}

		if ((service->id >= FIRME_SERVICE_ID_MAX) ||
		    (service->id != i) || (service->version == NULL) ||
		    (service->is_supported == NULL) ||
		    (service->get_feature_reg == NULL) ||
		    (service->call == NULL)) {
			ERROR("Invalid FIRME service table entry at index %u.\n",
			      i);
			return -EINVAL;
		}

		/* Perform any boot time initialisation if required. */
		if (service->init != NULL) {
			rc = service->init();
			if (rc != 0) {
				ERROR("FIRME service %u init failed (%d).\n",
				      service->id, rc);
				return rc;
			}
		}
	}

	return 0;
}

static const struct firme_service *
firme_get_service(firme_service_id_e service_id)
{
	if (service_id >= FIRME_SERVICE_ID_MAX) {
		VERBOSE("FIRME ABI 0x%X is not supported.\n", service_id);
		return NULL;
	}

	return firme_services[service_id];
}

bool firme_service_is_supported(firme_service_id_e service_id,
				firme_instance_e instance)
{
	const struct firme_service *service = firme_get_service(service_id);

	if ((service == NULL) || (service->is_supported == NULL)) {
		return false;
	}

	return service->is_supported(instance);
}

int32_t firme_service_get_version(firme_service_id_e service_id,
				  firme_instance_e instance)
{
	const struct firme_service *service = firme_get_service(service_id);

	if ((service == NULL) || !service->is_supported(instance)) {
		return FIRME_NOT_SUPPORTED;
	}

	return service->version(instance);
}

int32_t firme_service_get_feature_reg(firme_service_id_e service_id,
				      firme_instance_e instance,
				      uint8_t reg_index, uint64_t *reg)
{
	const struct firme_service *service = firme_get_service(service_id);

	if (reg == NULL) {
		return FIRME_INVALID_PARAMETERS;
	}

	if ((service == NULL) || !service->is_supported(instance)) {
		return FIRME_NOT_SUPPORTED;
	}

	return service->get_feature_reg(instance, reg_index, reg);
}

uint64_t firme_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3,
		       uint64_t x4, void *cookie, void *handle, uint64_t flags)
{
	const struct firme_service *service;
	firme_service_id_e service_id = get_service_id_from_fid(smc_fid);
	firme_instance_e instance = get_instance_from_flags(flags);

	service = firme_get_service(service_id);
	if ((service == NULL) || !service->is_supported(instance)) {
		VERBOSE("FIRME service %u is not available for ABI 0x%X.\n",
			service_id, smc_fid);
		SMC_RET1(handle, FIRME_NOT_SUPPORTED);
	}

	return service->call(instance, smc_fid, x1, x2, x3, x4, cookie, handle,
			     flags);
}
