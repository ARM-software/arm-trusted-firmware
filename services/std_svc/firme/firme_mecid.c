/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <services/firme_svc.h>
#include <smccc_helpers.h>

/* Only supported ABI is MEC_REFRESH for Realm callers. */
static firme_service_info_t mecid_info = {
	.version = FIRME_VERSION(FIRME_MECID_MGMT_VERSION_MAJOR,
				 FIRME_MECID_MGMT_VERSION_MINOR),
	.instance_support = BIT(FIRME_REALM),
	.num_feature_regs = FIRME_MECID_FEATURE_REG_COUNT,
	.feature_reg = { FIRME_MECID_FEAT_REG0_MEC_REFRESH_BIT },
};

firme_service_info_t *firme_mecid_service_get_info(void)
{
	if (!is_feat_mec_supported()) {
		return NULL;
	}

	return &mecid_info;
}

static int firme_mec_refresh(u_register_t mec_params)
{
	uint64_t common_mecid_width;
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
	common_mecid_width = ((read_mecidr_el2() >> MECIDR_EL2_MECIDWidthm1_SHIFT) &
		MECIDR_EL2_MECIDWidthm1_MASK) + 1UL;
	mecid = EXTRACT(MEC_PARAM_MECID, mec_params);

	if (mecid > common_mecid_width) {
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
