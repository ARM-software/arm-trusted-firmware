/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <common/debug.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/smccc.h>
#include <services/firme/firme_granule_mgmt.h>
#include <services/firme_svc.h>
#include <smccc_helpers.h>

/* Only supported ABI is GPI_SET for now. */
#define FEAT_REG_0_DEFAULT (FIRME_GM_GPI_SET_BIT)

/* Reg 1 is built at runtime based on hardware config. */
#define FEAT_REG_1_DEFAULT 0

static firme_service_info_t granule_mgmt_info = {
	.version = FIRME_VERSION(FIRME_GRANULE_MGMT_VERSION_MAJOR,
				 FIRME_GRANULE_MGMT_VERSION_MINOR),
#if ENABLE_RMM
	.instance_support =
		(BIT(FIRME_SECURE) | BIT(FIRME_NONSECURE) | BIT(FIRME_REALM)),
#else
	.instance_support = (BIT(FIRME_SECURE) | BIT(FIRME_NONSECURE)),
#endif
	.num_feature_regs = 2,
	.feature_reg = { FEAT_REG_0_DEFAULT, FEAT_REG_1_DEFAULT },
};

firme_service_info_t *firme_granule_mgmt_service_get_info(void)
{
	/* Build feat reg 1 value from GPCCR value. */
	uint64_t gpccr;

	if (!is_feat_rme_supported()) {
		return NULL;
	}

	/*
	 * todo: FIRME must have a init routine to perform one time
	 * initialization during boot for all supported services.
	 */
	gpccr = read_gpccr_el3();

	granule_mgmt_info.feature_reg[1] |=
		((gpccr >> GPCCR_PGS_SHIFT) & GPCCR_PGS_MASK)
		<< FIRME_GM_PGS_SHIFT;
	granule_mgmt_info.feature_reg[1] |=
		((gpccr >> GPCCR_L0GPTSZ_SHIFT) & GPCCR_L0GPTSZ_MASK)
		<< FIRME_GM_L0GPTSZ_SHIFT;
	granule_mgmt_info.feature_reg[1] |=
		((gpccr >> GPCCR_PPS_SHIFT) & GPCCR_PPS_MASK)
		<< FIRME_GM_PPS_SHIFT;

	return &granule_mgmt_info;
}

u_register_t firme_granule_mgmt_service_handler(firme_instance_e instance,
						uint32_t smc_fid, uint64_t x1,
						uint64_t x2, uint64_t x3,
						uint64_t x4, void *cookie,
						void *handle, uint64_t flags)
{
	if (!is_feat_rme_supported()) {
		SMC_RET1(handle, FIRME_NOT_SUPPORTED);
	}

	switch (smc_fid) {
	case FIRME_GM_GPI_SET_FID:
		/* Extract target GPI value from attributes in x3. */
		uint8_t target_gpi = (x3 >> FIRME_GM_GPI_SET_TGT_GPI_SHIFT) &
				     FIRME_GM_GPI_SET_TGT_GPI_MASK;
		/*
		 * Granule count is the number of granules to transition, and
		 * will be overwritten to contain the number of granules
		 * actually transitioned by gpt_transition_pas.
		 */
		uint64_t granule_count = x2;
		uint32_t ret = gpt_transition_pas(x1, &granule_count,
						  target_gpi,
						  caller_sec_state(flags));

		switch (ret) {
		case 0:
			SMC_RET2(handle, FIRME_SUCCESS, granule_count);
		case -EINVAL:
			SMC_RET2(handle, FIRME_INVALID_PARAMETERS, 0);
		case -EPERM:
			SMC_RET2(handle, FIRME_DENIED, 0);
		default:
			SMC_RET2(handle, FIRME_NOT_SUPPORTED, 0);
		}
		break;
	default:
		ERROR("FIRME Granule Management Service FID 0x%X not implemented\n",
		      smc_fid);
		SMC_RET1(handle, FIRME_NOT_SUPPORTED);
	}
}
