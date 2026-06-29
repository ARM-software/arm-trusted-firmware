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

/* Feature reg 0 indicates which ABIs are supported for base service. */
#define FEAT_REG_0_DEFAULT (FIRME_BASE_VERSION_BIT | FIRME_BASE_FEATURES_BIT)

/* Feature reg 1 shows available services and base functionalities. */
#define FEAT_REG_1_DEFAULT					\
	((0x0 & FIRME_BASE_MAX_SH_BUF_PG_CNT_MASK))		\
			<< FIRME_BASE_MAX_SH_BUF_PG_CNT_SHIFT |	\
		((0x0 & FIRME_BASE_MIN_SH_BUF_SZ_MASK)		\
		 << FIRME_BASE_MIN_SH_BUF_SZ_SHIFT)

/* Structure describing base service. */
firme_service_info_t base_info = {
	.version = FIRME_VERSION(FIRME_BASE_VERSION_MAJOR,
				 FIRME_BASE_VERSION_MINOR),
	.instance_support =
		(BIT(FIRME_SECURE) | BIT(FIRME_NONSECURE) | BIT(FIRME_REALM)),
	.num_feature_regs = 2,
	.feature_reg = { FEAT_REG_0_DEFAULT, FEAT_REG_1_DEFAULT },
};

static uint64_t firme_base_get_feat_reg_1(firme_instance_e instance)
{
	firme_service_info_t *info __unused;
	uint64_t reg = base_info.feature_reg[1];

	info = firme_granule_mgmt_service_get_info();
	if ((info != NULL) &&
	    ((info->instance_support & BIT(instance)) != 0U)) {
		reg |= FIRME_BASE_SERVICE_GRANULE_MGMT_BIT;
	}

	info = firme_mecid_service_get_info();
	if ((info != NULL) &&
	    ((info->instance_support & BIT(instance)) != 0U)) {
		reg |= FIRME_BASE_SERVICE_MECID_BIT;
	}

	return reg;
}

static int32_t get_firme_service_version(firme_instance_e instance,
					 firme_service_id_e service_id)
{
	firme_service_info_t *info = NULL;

	if (service_id >= FIRME_SERVICE_ID_MAX) {
		return FIRME_NOT_SUPPORTED;
	}

	switch (service_id) {
	case FIRME_BASE_ID:
		info = &base_info;
		break;
	case FIRME_GRANULE_MGMT_ID:
		info = firme_granule_mgmt_service_get_info();
		break;
	case FIRME_MECID_MGMT_ID:
		info = firme_mecid_service_get_info();
		break;
	default:
		return FIRME_NOT_SUPPORTED;
	}

	if ((info != NULL) && (info->instance_support & BIT(instance))) {
		return info->version;
	}

	/* Return zero to indicate not supported. */
	return FIRME_NOT_SUPPORTED;
}

static int32_t get_firme_feature_reg(uint64_t *reg, firme_instance_e instance,
				     uint8_t service_id, uint8_t reg_index)
{
	firme_service_info_t *info = NULL;

	if (service_id >= FIRME_SERVICE_ID_MAX) {
		return FIRME_NOT_SUPPORTED;
	}

	switch (service_id) {
	case FIRME_BASE_ID:
		if (reg_index < base_info.num_feature_regs) {
			if (reg_index == 1) {
				*reg = firme_base_get_feat_reg_1(instance);
			} else {
				*reg = base_info.feature_reg[reg_index];
			}
			return FIRME_SUCCESS;
		}
		break;
	case FIRME_GRANULE_MGMT_ID:
		info = firme_granule_mgmt_service_get_info();
		break;
	case FIRME_MECID_MGMT_ID:
		info = firme_mecid_service_get_info();
		break;
	}

	if ((info != NULL) && (reg_index < info->num_feature_regs) &&
	    (info->instance_support & BIT(instance))) {
		*reg = info->feature_reg[reg_index];
		return FIRME_SUCCESS;
	}

	return FIRME_NOT_SUPPORTED;
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
