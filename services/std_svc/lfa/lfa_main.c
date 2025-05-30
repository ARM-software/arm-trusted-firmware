/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>
#include <services/bl31_lfa.h>
#include <services/lfa_svc.h>
#include <services/rmmd_rmm_lfa.h>
#include <smccc_helpers.h>

static uint32_t lfa_component_count;
static plat_lfa_component_info_t *lfa_components;
static struct lfa_component_status current_activation;
static bool is_lfa_initialized;

void lfa_reset_activation(void)
{
	current_activation.component_id = LFA_INVALID_COMPONENT;
	current_activation.prime_status = PRIME_NONE;
}

static bool lfa_initialize_components(void)
{
	lfa_component_count = plat_lfa_get_components(&lfa_components);

	if (lfa_component_count == 0U || lfa_components == NULL) {
		/* unlikely to reach here */
		ERROR("Invalid LFA component setup: count = 0 or components are NULL");
		return false;
	}

	return true;
}

int lfa_setup(void)
{
	is_lfa_initialized = lfa_initialize_components();
	if (!is_lfa_initialized) {
		return -1;
	}

	lfa_reset_activation();

	return 0;
}

uint64_t lfa_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			 u_register_t x3, u_register_t x4, void *cookie,
			 void *handle, u_register_t flags)
{
	/**
	 * TODO: Acquire serialization lock.
	 */

	if (!is_lfa_initialized) {
		return LFA_NOT_SUPPORTED;
	}

	switch (smc_fid) {
	case LFA_VERSION:
		SMC_RET1(handle, LFA_VERSION_VAL);
		break;

	case LFA_FEATURES:
		SMC_RET1(handle, is_lfa_fid(x1) ? LFA_SUCCESS : LFA_NOT_SUPPORTED);
		break;

	case LFA_GET_INFO:
		/**
		 * The current specification limits this input parameter to be zero for
		 * version 1.0 of LFA
		 */
		if (x1 == 0ULL) {
			SMC_RET3(handle, LFA_SUCCESS, lfa_component_count, 0);
		} else {
			SMC_RET1(handle, LFA_INVALID_PARAMETERS);
		}
		break;

	case LFA_GET_INVENTORY:
		break;

	case LFA_PRIME:
		break;

	case LFA_ACTIVATE:
		break;

	case LFA_CANCEL:
		break;

	default:
		WARN("Unimplemented LFA Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break; /* unreachable */

	}

	SMC_RET1(handle, SMC_UNK);

	return 0;
}
