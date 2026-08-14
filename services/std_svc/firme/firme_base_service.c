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

#include "firme_private.h"

/*
 * Feature reg 0 indicates which ABIs are supported for base service.
 * Feature reg 1 shows available services and base functionalities.
 */
static uint64_t registers[FIRME_BASE_FEATURE_REG_COUNT] = {
	FIRME_BASE_VERSION_BIT | FIRME_BASE_FEATURES_BIT,
	(((0x0 & FIRME_BASE_MAX_SH_BUF_PG_CNT_MASK))
		 << FIRME_BASE_MAX_SH_BUF_PG_CNT_SHIFT |
	 ((0x0 & FIRME_BASE_MIN_SH_BUF_SZ_MASK)
	  << FIRME_BASE_MIN_SH_BUF_SZ_SHIFT))
};

#define BASE_INSTANCE_SUPPORT \
	(BIT(FIRME_SECURE) | BIT(FIRME_NONSECURE) | BIT(FIRME_REALM))

static int32_t firme_base_service_version(firme_instance_e instance __unused)
{
	return FIRME_VERSION(FIRME_BASE_VERSION_MAJOR,
			     FIRME_BASE_VERSION_MINOR);
}

static bool firme_base_service_is_supported(firme_instance_e instance)
{
	return (BASE_INSTANCE_SUPPORT & BIT(instance)) != 0U;
}

static int32_t firme_base_service_get_feature_reg(firme_instance_e instance,
						  uint8_t reg_index,
						  uint64_t *reg)
{
	if (reg == NULL) {
		return FIRME_INVALID_PARAMETERS;
	}

	if (reg_index >= FIRME_BASE_FEATURE_REG_COUNT) {
		return FIRME_NOT_SUPPORTED;
	}

	*reg = registers[reg_index];

	if (reg_index == 1) {
		if (firme_service_is_supported(FIRME_GRANULE_MGMT_ID,
					       instance)) {
			*reg |= FIRME_BASE_SERVICE_GRANULE_MGMT_BIT;
		}

		if (firme_service_is_supported(FIRME_MECID_MGMT_ID, instance)) {
			*reg |= FIRME_BASE_SERVICE_MECID_BIT;
		}

		if (firme_service_is_supported(FIRME_IDE_KEY_MGMT_ID,
					       instance)) {
			*reg |= FIRME_BASE_SERVICE_IDE_KM_BIT;
		}
	}

	return FIRME_SUCCESS;
}

static int32_t get_firme_service_version(firme_instance_e instance,
					 firme_service_id_e service_id)
{
	if (service_id == FIRME_BASE_ID) {
		return firme_base_service_version(instance);
	}

	return firme_service_get_version(service_id, instance);
}

static int32_t get_firme_feature_reg(uint64_t *reg, firme_instance_e instance,
				     uint8_t service_id, uint8_t reg_index)
{
	if (reg == NULL) {
		return FIRME_INVALID_PARAMETERS;
	}

	if (service_id >= FIRME_SERVICE_ID_MAX) {
		return FIRME_NOT_SUPPORTED;
	}

	if (service_id == FIRME_BASE_ID) {
		return firme_base_service_get_feature_reg(instance, reg_index,
							  reg);
	}

	return firme_service_get_feature_reg(service_id, instance, reg_index,
					     reg);
}

u_register_t firme_base_service_handler(firme_instance_e instance,
					uint32_t smc_fid, uint64_t x1,
					uint64_t x2, uint64_t x3, uint64_t x4,
					void *cookie, void *handle,
					uint64_t flags)
{
	uint64_t reg;

	switch (smc_fid) {
	case FIRME_SERVICE_VERSION_FID:
		SMC_RET1(handle,
			 get_firme_service_version(instance, (uint8_t)x1));
		break;
	case FIRME_SERVICE_FEATURES_FID:
		if (get_firme_feature_reg(&reg, instance, (uint8_t)x1,
					  (uint8_t)x2) == FIRME_SUCCESS) {
			SMC_RET2(handle, FIRME_SUCCESS, reg);
		} else {
			SMC_RET1(handle, FIRME_NOT_SUPPORTED);
		}
		break;
	default:
		ERROR("FIRME Base Service FID 0x%X not implemented\n", smc_fid);
		SMC_RET1(handle, FIRME_NOT_SUPPORTED);
	}
}

const struct firme_service firme_base_service = {
	.id = FIRME_BASE_ID,
	.init = NULL,
	.version = firme_base_service_version,
	.is_supported = firme_base_service_is_supported,
	.get_feature_reg = firme_base_service_get_feature_reg,
	.call = firme_base_service_handler,
};
