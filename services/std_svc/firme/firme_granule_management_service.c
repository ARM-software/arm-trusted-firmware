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

#include "firme_private.h"

/* Only supported ABI is GPI_SET for now. */
static uint64_t registers[FIRME_GRANULE_MGMT_FEATURE_REG_COUNT] = {
	FIRME_GM_GPI_SET_BIT, 0U
};


#if ENABLE_RMM
#define GRANULE_MGMT_INSTANCE_SUPPORT \
	(BIT(FIRME_SECURE) | BIT(FIRME_NONSECURE) | BIT(FIRME_REALM))
#else
#define GRANULE_MGMT_INSTANCE_SUPPORT \
	(BIT(FIRME_SECURE) | BIT(FIRME_NONSECURE))
#endif /* ENABLE_RMM */


static int32_t firme_granule_mgmt_service_init(void)
{
	/* Build feat reg 1 value from GPCCR value. */
	uint64_t gpccr;

	registers[1] = 0U;

	if (is_feat_rme_supported()) {
		gpccr = read_gpccr_el3();

		registers[1] |= ((gpccr >> GPCCR_PGS_SHIFT) & GPCCR_PGS_MASK)
				<< FIRME_GM_PGS_SHIFT;
		registers[1] |=
			((gpccr >> GPCCR_L0GPTSZ_SHIFT) & GPCCR_L0GPTSZ_MASK)
			<< FIRME_GM_L0GPTSZ_SHIFT;
		registers[1] |= ((gpccr >> GPCCR_PPS_SHIFT) & GPCCR_PPS_MASK)
				<< FIRME_GM_PPS_SHIFT;
	}

	return FIRME_SUCCESS;
}

static int32_t
firme_granule_mgmt_service_version(firme_instance_e instance __unused)
{
	return FIRME_VERSION(FIRME_GRANULE_MGMT_VERSION_MAJOR,
			     FIRME_GRANULE_MGMT_VERSION_MINOR);
}

static bool firme_granule_mgmt_service_is_supported(firme_instance_e instance)
{
	return is_feat_rme_supported() &&
	       (GRANULE_MGMT_INSTANCE_SUPPORT & BIT(instance)) != 0U;
}

static int32_t
firme_granule_mgmt_service_get_feature_reg(firme_instance_e instance __unused,
					   uint8_t reg_index, uint64_t *reg)
{
	if (reg == NULL) {
		return FIRME_INVALID_PARAMETERS;
	}

	if (reg_index >= FIRME_GRANULE_MGMT_FEATURE_REG_COUNT) {
		return FIRME_NOT_SUPPORTED;
	}

	*reg = registers[reg_index];
	return FIRME_SUCCESS;
}

/*
 * This temporarily calls legacy lib/gpt helpers to set one granule until FIRME
 * granule management supports native GPI transistion for multiple granules with
 * stateful long running operations.
 */
static uint32_t gm_gpi_set(uint64_t base, uint64_t gcnt, uint64_t attrs,
			   uint64_t flags, uint64_t *gcnt_ret)
{
	uint32_t ret;
	uint8_t target_gpi;
	uint32_t src_sec_state = caller_sec_state(flags);

	/* gpi set currently supports one granule  */
	if (gcnt != 1U) {
		return -EINVAL;
	}

	/* Extract target GPI value from attributes in x3. */
	target_gpi = (attrs >> FIRME_GM_GPI_SET_TGT_GPI_SHIFT) &
		FIRME_GM_GPI_SET_TGT_GPI_MASK;

	/*
	 * Invoke GPI set implementation in legacy lib GPT until FIRME supports
	 * native granule transition.
	 */
	ret = gpt_firme_gpi_set(base, target_gpi, src_sec_state);
	if (ret == 0) {
		*gcnt_ret = 1U;
	}

	return ret;
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
	case FIRME_GM_GPI_SET_FID: {
		uint32_t ret;
		uint64_t gcnt = 0;

		ret = gm_gpi_set(x1, x2, x3, flags, &gcnt);
		switch (ret) {
		case 0:
			SMC_RET2(handle, FIRME_SUCCESS, gcnt);
		case -EINVAL:
			SMC_RET2(handle, FIRME_INVALID_PARAMETERS, 0);
		case -EPERM:
			SMC_RET2(handle, FIRME_DENIED, 0);
		default:
			SMC_RET2(handle, FIRME_NOT_SUPPORTED, 0);
		}
		break;
	}
	default:
		ERROR("FIRME Granule Management Service FID 0x%X not implemented\n",
		      smc_fid);
		SMC_RET1(handle, FIRME_NOT_SUPPORTED);
	}
}

const struct firme_service firme_granule_mgmt_service = {
	.id = FIRME_GRANULE_MGMT_ID,
	.init = firme_granule_mgmt_service_init,
	.version = firme_granule_mgmt_service_version,
	.is_supported = firme_granule_mgmt_service_is_supported,
	.get_feature_reg = firme_granule_mgmt_service_get_feature_reg,
	.call = firme_granule_mgmt_service_handler,
};
