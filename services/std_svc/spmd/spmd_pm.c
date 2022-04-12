/*
 * Copyright (c) 2020-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>

#include <lib/el3_runtime/context_mgmt.h>
#include <lib/spinlock.h>
#include "spmd_private.h"

static struct {
	bool secondary_ep_locked;
	uintptr_t secondary_ep;
	spinlock_t lock;
} g_spmd_pm;

/*******************************************************************************
 * spmd_pm_secondary_ep_register
 ******************************************************************************/
int spmd_pm_secondary_ep_register(uintptr_t entry_point)
{
	int ret = FFA_ERROR_INVALID_PARAMETER;

	spin_lock(&g_spmd_pm.lock);

	if (g_spmd_pm.secondary_ep_locked == true) {
		goto out;
	}

	/*
	 * Check entry_point address is a PA within
	 * load_address <= entry_point < load_address + binary_size
	 */
	if (!spmd_check_address_in_binary_image(entry_point)) {
		ERROR("%s entry point is not within image boundaries\n",
			__func__);
		goto out;
	}

	g_spmd_pm.secondary_ep = entry_point;
	g_spmd_pm.secondary_ep_locked = true;

	VERBOSE("%s %lx\n", __func__, entry_point);

	ret = 0;

out:
	spin_unlock(&g_spmd_pm.lock);

	return ret;
}

/*******************************************************************************
 * This CPU has been turned on. Enter SPMC to initialise S-EL1 or S-EL2. As part
 * of the SPMC initialization path, they will initialize any SPs that they
 * manage. Entry into SPMC is done after initialising minimal architectural
 * state that guarantees safe execution.
 ******************************************************************************/
static void spmd_cpu_on_finish_handler(u_register_t unused)
{
	spmd_spm_core_context_t *ctx = spmd_get_context();
	unsigned int linear_id = plat_my_core_pos();
	el3_state_t *el3_state;
	uintptr_t entry_point;
	uint64_t rc;

	assert(ctx != NULL);
	assert(ctx->state != SPMC_STATE_ON);

	spin_lock(&g_spmd_pm.lock);

	/*
	 * Leave the possibility that the SPMC does not call
	 * FFA_SECONDARY_EP_REGISTER in which case re-use the
	 * primary core address for booting secondary cores.
	 */
	if (g_spmd_pm.secondary_ep_locked == true) {
		/*
		 * The CPU context has already been initialized at boot time
		 * (in spmd_spmc_init by a call to cm_setup_context). Adjust
		 * below the target core entry point based on the address
		 * passed to by FFA_SECONDARY_EP_REGISTER.
		 */
		entry_point = g_spmd_pm.secondary_ep;
		el3_state = get_el3state_ctx(&ctx->cpu_ctx);
		write_ctx_reg(el3_state, CTX_ELR_EL3, entry_point);
	}

	spin_unlock(&g_spmd_pm.lock);

	/* Mark CPU as initiating ON operation. */
	ctx->state = SPMC_STATE_ON_PENDING;

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0ULL) {
		ERROR("%s failed (%" PRIu64 ") on CPU%u\n", __func__, rc,
			linear_id);
		ctx->state = SPMC_STATE_OFF;
		return;
	}

	ctx->state = SPMC_STATE_ON;

	VERBOSE("CPU %u on!\n", linear_id);
}

/*******************************************************************************
 * spmd_cpu_off_handler
 ******************************************************************************/
static int32_t spmd_cpu_off_handler(u_register_t unused)
{
	spmd_spm_core_context_t *ctx = spmd_get_context();
	unsigned int linear_id = plat_my_core_pos();
	int64_t rc;

	assert(ctx != NULL);
	assert(ctx->state != SPMC_STATE_OFF);

	/* Build an SPMD to SPMC direct message request. */
	spmd_build_spmc_message(get_gpregs_ctx(&ctx->cpu_ctx),
				FFA_FWK_MSG_PSCI, PSCI_CPU_OFF);

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0ULL) {
		ERROR("%s failed (%" PRIu64 ") on CPU%u\n", __func__, rc, linear_id);
	}

	/* Expect a direct message response from the SPMC. */
	u_register_t ffa_resp_func = read_ctx_reg(get_gpregs_ctx(&ctx->cpu_ctx),
						  CTX_GPREG_X0);
	if (ffa_resp_func != FFA_MSG_SEND_DIRECT_RESP_SMC32) {
		ERROR("%s invalid SPMC response (%lx).\n",
			__func__, ffa_resp_func);
		return -EINVAL;
	}

	ctx->state = SPMC_STATE_OFF;

	VERBOSE("CPU %u off!\n", linear_id);

	return 0;
}

/*******************************************************************************
 * Structure populated by the SPM Dispatcher to perform any bookkeeping before
 * PSCI executes a power mgmt. operation.
 ******************************************************************************/
const spd_pm_ops_t spmd_pm = {
	.svc_on_finish = spmd_cpu_on_finish_handler,
	.svc_off = spmd_cpu_off_handler
};
