/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 * Copyright (c) 2025, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <services/bl31_lfa.h>
#include <services/lfa_svc.h>
#include <services/rmmd_rmm_lfa.h>
#include <smccc_helpers.h>

static plat_lfa_component_info_t *lfa_components;
static struct lfa_component_status current_activation;

/*
 * Multiple cores call into LFA service which shares these variables, so ensure
 * that any accesses to these variables go back to memory. These are sort of
 * "lazy" state variables where timing isn't really critical so we aren't
 * explicitly using mutex for access control but we just want to make sure
 * nothing is optimized away.
 */
static volatile uint32_t lfa_component_count;
static volatile bool is_lfa_initialized;
static volatile bool activation_in_progress;
static volatile bool activation_skip_cpu_rendezvous;
static volatile uint32_t activation_users;
static volatile bool activation_failed;

/*
 * Spinlock to serialize LFA operations (PRIME, ACTIVATE).
 * This ensures that these calls from different CPUs are properly
 * serialized and do not execute concurrently, while still allowing
 * the same operation to be invoked from any CPU.
 */
static spinlock_t lfa_lock;

void lfa_reset_activation(void)
{
	current_activation.component_id = LFA_INVALID_COMPONENT;
	current_activation.prime_status = PRIME_NONE;
	current_activation.cpu_rendezvous = false;
	activation_in_progress = false;
	activation_skip_cpu_rendezvous = false;
	activation_users = 0U;
	activation_failed = false;
}

static int convert_to_lfa_error(int ret)
{
	switch (ret) {
	case 0:
		return LFA_SUCCESS;
	case -EAUTH:
		return LFA_AUTH_ERROR;
	case -ENOMEM:
		return LFA_NO_MEMORY;
	default:
		return LFA_DEVICE_ERROR;
	}
}

static bool lfa_initialize_components(void)
{
	lfa_component_count = plat_lfa_get_components(&lfa_components);

	if (lfa_component_count == 0U || lfa_components == NULL) {
		ERROR("Could not retrieve LFA components.\n");
		return false;
	}

	return true;
}

static uint64_t get_fw_activation_flags(uint32_t fw_seq_id)
{
	const plat_lfa_component_info_t *comp = &lfa_components[fw_seq_id];
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
	struct lfa_component_ops *activator;

	if (lfa_component_count == 0U) {
		return LFA_WRONG_STATE;
	}

	/* Check if component ID is in range. */
	if ((component_id >= lfa_component_count) ||
	    (component_id != current_activation.component_id)) {
		return LFA_INVALID_PARAMETERS;
	}

	activator = lfa_components[component_id].activator;
	if (activator->cancel != NULL) {
		ret = activator->cancel(&current_activation);
		if (ret != LFA_SUCCESS) {
			return LFA_BUSY;
		}
	}

	ret = plat_lfa_cancel(component_id);
	if (ret != LFA_SUCCESS) {
		return LFA_BUSY;
	}

	/* TODO: add proper termination prime and activate phases */
	lfa_reset_activation();

	return ret;
}

static int lfa_prime(uint32_t component_id, uint64_t *flags)
{
	int ret = LFA_SUCCESS;
	struct lfa_component_ops *activator;

	/* Check if fw_seq_id is in range. */
	if (component_id >= lfa_component_count) {
		return LFA_INVALID_PARAMETERS;
	}

	if (lfa_component_count == 0U ||
	    !lfa_components[component_id].activation_pending) {
		return LFA_WRONG_STATE;
	}

	if (lfa_components[component_id].activator == NULL) {
		return LFA_NOT_SUPPORTED;
	}

	switch (current_activation.prime_status) {
	case PRIME_NONE:
		current_activation.component_id = component_id;
		current_activation.prime_status = PRIME_STARTED;
		break;

	case PRIME_STARTED:
		if (current_activation.component_id != component_id) {
			/* Mismatched component trying to continue PRIME - error */
			return LFA_WRONG_STATE;
		}
		break;

	case PRIME_COMPLETE:
	default:
		break;
	}

	/* Initialise the flags to start with. Only valid if ret=LFA_SUCCESS. */
	*flags = 0ULL;

	ret = plat_lfa_load_auth_image(component_id);
	if (ret == 0) {
		activator = lfa_components[component_id].activator;
		if (activator->prime != NULL) {
			ret = activator->prime(&current_activation);
			if (ret != LFA_SUCCESS) {
				/*
				* TODO: it should be LFA_PRIME_FAILED but specification
				* has not define this error yet
				*/
				return ret;
			}
		}

		/*
		 * Update current activation status fields. CPU rendezvous is enabled
		 * by default but can be disabled if requested and allowed.
		 */
		current_activation.cpu_rendezvous = true;
		current_activation.prime_status = PRIME_COMPLETE;
		current_activation.reset = lfa_components[component_id].activator->may_reset_cpu;
	}

	/*
	 * Set lfa_flags to indicate that LFA_PRIME must be called again and
	 * reset ret to 0, as LFA_PRIME must return LFA_SUCCESS if it is
	 * incomplete.
	 */
	if (ret == -EAGAIN) {
		ret = 0;
		*flags = LFA_CALL_AGAIN;
	}

	return convert_to_lfa_error(ret);
}

bool lfa_is_prime_complete(uint32_t lfa_component_id)
{
	if (lfa_component_id >= lfa_component_count) {
		return false;
	}

	return (current_activation.component_id == lfa_component_id &&
		current_activation.prime_status == PRIME_COMPLETE &&
		lfa_components[lfa_component_id].activation_pending == true);
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

static int lfa_activate_prepare(uint32_t component_id, uint64_t flags,
				struct lfa_component_ops **activator)
{
	int ret = LFA_ACTIVATION_FAILED;
	/* First caller decides activation policy and performs platform notify. */
	bool first_cpu = !activation_in_progress;

	/* Check if fw_seq_id is in range. */
	if ((component_id >= lfa_component_count) ||
	    (current_activation.component_id != component_id)) {
		return LFA_INVALID_PARAMETERS;
	}

	if ((lfa_component_count == 0U) ||
	    (!lfa_components[component_id].activation_pending) ||
	    (current_activation.prime_status != PRIME_COMPLETE)) {
		return LFA_COMPONENT_WRONG_STATE;
	}

	if (lfa_components[component_id].activator == NULL) {
		return LFA_NOT_SUPPORTED;
	}

	*activator = lfa_components[component_id].activator;

	if (first_cpu) {
		activation_failed = false;

		/*
		 * If rendezvous is optional for this component, default to
		 * rendezvous unless caller explicitly asks to skip it.
		 * This choice is latched for the whole activation.
		 */
		if (!(*activator)->cpu_rendezvous_required &&
		    ((flags & LFA_SKIP_CPU_RENDEZVOUS_BIT) == 0U)) {
			current_activation.cpu_rendezvous = true;
		} else {
			current_activation.cpu_rendezvous =
				(*activator)->cpu_rendezvous_required;
		}
		activation_skip_cpu_rendezvous = false;
	}
	/*
	 * Pass skip_cpu_rendezvous (flag[0]) only if flag[0]==1
	 * & CPU_RENDEZVOUS is not required.
	 */
	if (flags & LFA_SKIP_CPU_RENDEZVOUS_BIT) {
		if (!(*activator)->cpu_rendezvous_required) {
			/*
			 * Late skip request is rejected if activation already
			 * started without skip.
			 */
			if (!first_cpu && !activation_skip_cpu_rendezvous) {
				return LFA_BUSY;
			}
			INFO("Skipping rendezvous requested by caller.\n");
			current_activation.cpu_rendezvous = false;
			activation_skip_cpu_rendezvous = true;
		}
		/*
		 * Return error if caller tries to skip rendezvous when
		 * it is required.
		 */
		else {
			ERROR("CPU Rendezvous is required, can't skip.\n");
			return LFA_INVALID_PARAMETERS;
		}
	}

	if (first_cpu) {
		/* Notify platform once per activation round. */
		ret = plat_lfa_notify_activate(component_id);
		if (ret != 0) {
			return LFA_ACTIVATION_FAILED;
		}
		activation_in_progress = true;
	}

	/* Track how many CPUs have entered LFA_ACTIVATE for this round. */
	activation_users += 1U;

	return LFA_SUCCESS;
}


static void lfa_activate_finish(uint32_t component_id, bool activation_complete)
{
	if (activation_users > 0U) {
		activation_users -= 1U;
	}

	if (!activation_complete) {
		return;
	}

	if (activation_users == 0U) {
		activation_in_progress = false;
		activation_skip_cpu_rendezvous = false;
		if (!activation_failed) {
			lfa_components[component_id].activation_pending = false;
		}
	}
}

uint64_t lfa_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			 u_register_t x3, u_register_t x4, void *cookie,
			 void *handle, u_register_t flags)
{
	uint64_t retx1;
	uint64_t retx2;
	uint64_t lfa_flags = 0;
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
		/*
		 * The current specification limits this input parameter to be zero for
		 * version 1.0 of LFA.
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
		 * Check if fw_seq_id is in range. LFA_GET_INFO must be called
		 * first to scan platform firmware and create a valid number of
		 * firmware components.
		 */
		if (fw_seq_id >= lfa_component_count) {
			SMC_RET1(handle, LFA_INVALID_PARAMETERS);
		}

		/* Get the UUID of requested fw_seq_id. */
		uuid_p = (uint8_t *)&lfa_components[fw_seq_id].uuid;
		memcpy(&retx1, uuid_p, sizeof(uint64_t));
		memcpy(&retx2, uuid_p + sizeof(uint64_t), sizeof(uint64_t));

		/*
		 * Check the given fw_seq_id update available and accordingly
		 * set the active_pending flag.
		 */
		lfa_components[fw_seq_id].activation_pending =
				is_plat_lfa_activation_pending(fw_seq_id);

		INFO("Component %lu %s live activation.\n", x1,
		      lfa_components[fw_seq_id].activator ? "supports" :
		      "does not support");

		if (lfa_components[fw_seq_id].activator != NULL) {
			INFO("Activation pending: %s\n",
			      lfa_components[fw_seq_id].activation_pending ? "true" : "false");
		}

		SMC_RET4(handle, LFA_SUCCESS, retx1, retx2, get_fw_activation_flags(fw_seq_id));
		break;

	case LFA_PRIME:
		/*
		 * Acquire lock to serialize PRIME operations across CPUs.
		 * This ensures that multiple PRIME calls to the same component
		 * do not execute concurrently, even if issued from different
		 * CPUs.
		 */
		if (!spin_trylock(&lfa_lock)) {
			SMC_RET1(handle, LFA_BUSY);
		}

		ret = lfa_prime(x1, &lfa_flags);

		spin_unlock(&lfa_lock);

		if (ret != LFA_SUCCESS) {
			SMC_RET1(handle, ret);
		} else {
			SMC_RET2(handle, ret, lfa_flags);
		}
		break;

	case LFA_ACTIVATE:
		/* LFA_ACTIVATE flow:
		 * - LFA_SKIP_CPU_RENDEZVOUS_BIT controls skip request on entry.
		 * - Serialize with lfa_lock, but never block behind a
		 *   skip-rendezvous activation.
		 * - lfa_activate_prepare() validates state, selects activator,
		 *   and latches policy.
		 * - The first CPU decides rendezvous policy for the whole
		 *   activation.
		 * - Skip rendezvous keeps the lock for the full activation;
		 *   normal path releases it.
		 * - activator->activate() may return -EAGAIN, mapped to
		 *   LFA_CALL_AGAIN in lfa_flags.
		 * - lfa_activate_finish() updates bookkeeping and clears
		 *   in-progress state when done.
		 */
		bool hold_lock = false;
		bool activation_complete = true;
		struct lfa_component_ops *activator;
		/* Caller request: skip CPU rendezvous for this activation. */
		bool skip_cpu_rendezvous =
			((x2 & LFA_SKIP_CPU_RENDEZVOUS_BIT) != 0U);

		/*
		 * When skip_cpu_rendezvous=1 (or when a skip rendezvous
		 * activation is already in progress), concurrent LFA_ACTIVATE
		 * calls must not be accepted; return LFA_BUSY instead of
		 * blocking.
		 */
		if (skip_cpu_rendezvous) {
			/* Non-blocking serialization for skip rendezvous requests. */
			if (!spin_trylock(&lfa_lock)) {
				SMC_RET1(handle, LFA_BUSY);
			}
		} else {
			/*
			 * Normal path: take the lock, but reject if a skip
			 * activation is in progress.
			 */
			spin_lock(&lfa_lock);
			if (activation_skip_cpu_rendezvous) {
				spin_unlock(&lfa_lock);
				SMC_RET1(handle, LFA_BUSY);
			}
		}

		ret = lfa_activate_prepare(fw_seq_id, x2, &activator);
		hold_lock = (ret == LFA_SUCCESS) && activation_skip_cpu_rendezvous;

		/*
		 * Keep the lock held for the full activation only when skip
		 * rendezvous is in effect.
		 */
		if (!hold_lock) {
			spin_unlock(&lfa_lock);
		}

		if (ret != LFA_SUCCESS) {
			SMC_RET2(handle, ret, 0ULL);
		}

		if (activator->activate != NULL) {
			ret = activator->activate(&current_activation, x3, x4);
		}

		/* Reacquire lock before updating shared activation state. */
		if (!hold_lock) {
			spin_lock(&lfa_lock);
		}

		lfa_flags = 0ULL;
		if (ret == -EAGAIN) {
			/* Multi-stage activation: caller must reissue LFA_ACTIVATE. */
			ret = LFA_SUCCESS;
			lfa_flags = LFA_CALL_AGAIN;
			activation_complete = false;
		} else if (ret != LFA_SUCCESS) {
			activation_failed = true;
		}

		/* Update activation bookkeeping and clear in-progress state if complete. */
		lfa_activate_finish(fw_seq_id, activation_complete);

		spin_unlock(&lfa_lock);

		SMC_RET2(handle, ret, lfa_flags);

		break;

	case LFA_CANCEL:
		ret = lfa_cancel(x1);
		SMC_RET1(handle, ret);
		break;

	default:
		WARN("Unimplemented LFA Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break;
	}

	return 0;
}
