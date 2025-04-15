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

static uint64_t get_fw_activation_flags(uint32_t fw_seq_id)
{
	const plat_lfa_component_info_t *comp =
				&lfa_components[fw_seq_id];
	uint64_t flags = 0ULL;

	flags |= ((comp->activator == NULL ? 0ULL : 1ULL)
		 << LFA_ACTIVATION_CAPABLE_SHIFT);
	flags |= (uint64_t)(comp->activation_pending)
		 << LFA_ACTIVATION_PENDING_SHIFT;

	if (comp->activator != NULL) {
		flags |= ((comp->activator->may_reset_cpu ? 1ULL : 0ULL)
			 << LFA_MAY_RESET_CPU_SHIFT);
		flags |= ((comp->activator->cpu_rendezvous_required ? 0ULL : 1ULL)
			 << LFA_CPU_RENDEZVOUS_OPTIONAL_SHIFT);
	}

	return flags;
}

static int lfa_cancel(uint32_t component_id)
{
	int ret = LFA_SUCCESS;

	if (lfa_component_count == 0U) {
		return LFA_WRONG_STATE;
	}

	/* Check if component ID is in range. */
	if ((component_id >= lfa_component_count) ||
	    (component_id != current_activation.component_id)) {
		return LFA_INVALID_PARAMETERS;
	}

	ret = plat_lfa_cancel(component_id);
	if (ret != LFA_SUCCESS) {
		return LFA_BUSY;
	}

	/* TODO: add proper termination prime and activate phases */
	lfa_reset_activation();

	return ret;
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
	uint64_t retx1, retx2;
	uint8_t *uuid_p;
	uint32_t fw_seq_id = (uint32_t)x1;
	int ret;

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
		if (lfa_component_count == 0U) {
			SMC_RET1(handle, LFA_WRONG_STATE);
		}

		/*
		 * Check if fw_seq_id is in range. LFA_GET_INFO must be called first to scan
		 * platform firmware and create a valid number of firmware components.
		 */
		if (fw_seq_id >= lfa_component_count) {
			SMC_RET1(handle, LFA_INVALID_PARAMETERS);
		}

		/*
		 * grab the UUID of asked fw_seq_id and set the return UUID
		 * variables
		 */
		uuid_p = (uint8_t *)&lfa_components[fw_seq_id].uuid;
		memcpy(&retx1, uuid_p, sizeof(uint64_t));
		memcpy(&retx2, uuid_p + sizeof(uint64_t), sizeof(uint64_t));

		/*
		 * check the given fw_seq_id's update available
		 * and accordingly set the active_pending flag
		 */
		lfa_components[fw_seq_id].activation_pending =
				is_plat_lfa_activation_pending(fw_seq_id);

		INFO("Component %lu %s live activation:\n", x1,
		      lfa_components[fw_seq_id].activator ? "supports" :
		      "does not support");

		if (lfa_components[fw_seq_id].activator != NULL) {
			INFO("Activation pending: %s\n",
			      lfa_components[fw_seq_id].activation_pending ? "true" : "false");
		}

		INFO("x1 = 0x%016lx, x2 = 0x%016lx\n", retx1, retx2);

		SMC_RET4(handle, LFA_SUCCESS, retx1, retx2, get_fw_activation_flags(fw_seq_id));

		break;

	case LFA_PRIME:
		break;

	case LFA_ACTIVATE:
		break;

	case LFA_CANCEL:
		ret = lfa_cancel(x1);
		SMC_RET1(handle, ret);
		break;

	default:
		WARN("Unimplemented LFA Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break; /* unreachable */

	}

	SMC_RET1(handle, SMC_UNK);

	return 0;
}
