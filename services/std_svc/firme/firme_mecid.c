/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <services/firme/firme_mecid.h>
#include <services/firme_svc.h>
#include <smccc_helpers.h>

#include "firme_private.h"

/* Only supported ABI is MEC_REFRESH for Realm callers. */
static uint64_t registers[FIRME_MECID_FEATURE_REG_COUNT];

int32_t firme_mecid_service_init(void)
{
	uint64_t __maybe_unused mecid_width;

	registers[0] = 0U;
	registers[1] = 0U;

	if (is_feat_mec_supported()) {
		registers[0] = FIRME_MECID_FEAT_REG0_MEC_REFRESH_BIT;

		mecid_width = (uint64_t)plat_firme_get_common_mecid_width();
		if ((mecid_width &
		      ~FIRME_MECID_FEAT_REG1_COMMON_MECID_WIDTH_BITS_MASK) != 0U) {
			return FIRME_INVALID_PARAMETERS;
		}

		registers[1] =
			EXTRACT(FIRME_MECID_FEAT_REG1_COMMON_MECID_WIDTH_BITS,
				mecid_width);
	}

	return 0;
}

static int32_t firme_mecid_service_version(firme_instance_e instance __unused)
{
	return FIRME_VERSION(FIRME_MECID_MGMT_VERSION_MAJOR,
			     FIRME_MECID_MGMT_VERSION_MINOR);
}

static bool firme_mecid_service_is_supported(firme_instance_e instance)
{
	return (BIT(FIRME_REALM) & BIT(instance)) != 0U && (registers[0] != 0U);
}

static int32_t
firme_mecid_service_get_feature_reg(firme_instance_e instance __unused,
				    uint8_t reg_index, uint64_t *reg)
{
	if (reg == NULL) {
		return FIRME_INVALID_PARAMETERS;
	}

	if (reg_index >= FIRME_MECID_FEATURE_REG_COUNT) {
		return FIRME_NOT_SUPPORTED;
	}

	*reg = registers[reg_index];
	return FIRME_SUCCESS;
}

static int firme_mec_refresh(u_register_t mec_params)
{
	uint64_t common_mecid_width;
	uint64_t mecid_mask;
	uint16_t mecid;

	/*
	 * Check whether FEAT_MEC is supported by the hardware. If not, return
	 * unknown SMC.
	 */
	if (!is_feat_mec_supported()) {
		return FIRME_NOT_SUPPORTED;
	}

	/*
	 * Check whether the MECID parameter fits within the common MECID width.
	 */
	common_mecid_width =
		(registers[1] &
		 FIRME_MECID_FEAT_REG1_COMMON_MECID_WIDTH_BITS_MASK) +
		1U;
	mecid = EXTRACT(MEC_PARAM_MECID, mec_params);
	mecid_mask = (1UL << common_mecid_width) - 1UL;

	if ((mecid & ~mecid_mask) != 0U) {
		return FIRME_INVALID_PARAMETERS;
	}

	return plat_firme_mec_refresh(mecid, mec_params & BIT(0));
}

u_register_t firme_mecid_service_handler(firme_instance_e instance,
					 uint32_t smc_fid, uint64_t x1,
					 uint64_t x2, uint64_t x3,
					 uint64_t x4, void *cookie,
					 void *handle, uint64_t flags)
{
	(void)instance;
	(void)x2;
	(void)x3;
	(void)x4;
	(void)cookie;
	(void)handle;
	(void)flags;

	if (smc_fid != FIRME_MEC_REFRESH_FID || instance != FIRME_REALM) {
		SMC_RET1(handle, FIRME_NOT_SUPPORTED);
	}

	SMC_RET1(handle, firme_mec_refresh(x1));
}

const struct firme_service firme_mecid_service = {
	.id = FIRME_MECID_MGMT_ID,
	.version = firme_mecid_service_version,
	.init = firme_mecid_service_init,
	.is_supported = firme_mecid_service_is_supported,
	.get_feature_reg = firme_mecid_service_get_feature_reg,
	.call = firme_mecid_service_handler,
};
