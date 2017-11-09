/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <debug.h>
#include <string.h>
#include <tsp.h>
#include <utils.h>
#include "tspd_private.h"

/*******************************************************************************
 * Given a secure payload entrypoint info pointer, entry point PC, register
 * width, cpu id & pointer to a context data structure, this function will
 * initialize tsp context and entry point info for the secure payload
 ******************************************************************************/
void tspd_init_tsp_ep_state(struct entry_point_info *tsp_entry_point,
				uint32_t rw,
				uint64_t pc,
				tsp_context_t *tsp_ctx)
{
	uint32_t ep_attr;

	/* Passing a NULL context is a critical programming error */
	assert(tsp_ctx);
	assert(tsp_entry_point);
	assert(pc);

	/*
	 * We support AArch64 TSP for now.
	 * TODO: Add support for AArch32 TSP
	 */
	assert(rw == TSP_AARCH64);

	/* Associate this context with the cpu specified */
	tsp_ctx->mpidr = read_mpidr_el1();
	tsp_ctx->state = 0;
	set_tsp_pstate(tsp_ctx->state, TSP_PSTATE_OFF);
	clr_yield_smc_active_flag(tsp_ctx->state);

	cm_set_context(&tsp_ctx->cpu_ctx, SECURE);

	/* initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT)
		ep_attr |= EP_EE_BIG;
	SET_PARAM_HEAD(tsp_entry_point, PARAM_EP, VERSION_1, ep_attr);

	tsp_entry_point->pc = pc;
	tsp_entry_point->spsr = SPSR_64(MODE_EL1,
					MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS);
	zeromem(&tsp_entry_point->args, sizeof(tsp_entry_point->args));
}

/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Applies the S-EL1 system register context from tsp_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the tsp_ctx->cpu_ctx are used to enter the secure payload image.
 ******************************************************************************/
uint64_t tspd_synchronous_sp_entry(tsp_context_t *tsp_ctx)
{
	uint64_t rc;

	assert(tsp_ctx != NULL);
	assert(tsp_ctx->c_rt_ctx == 0);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context(SECURE) == &tsp_ctx->cpu_ctx);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = tspd_enter_sp(&tsp_ctx->c_rt_ctx);
#if ENABLE_ASSERTIONS
	tsp_ctx->c_rt_ctx = 0;
#endif

	return rc;
}


/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Saves the S-EL1 system register context tp tsp_ctx->cpu_ctx.
 * 2. Restores the current C runtime state (callee saved registers) from the
 *    stack frame using the reference to this state saved in tspd_enter_sp().
 * 3. It does not need to save any general purpose or EL3 system register state
 *    as the generic smc entry routine should have saved those.
 ******************************************************************************/
void tspd_synchronous_sp_exit(tsp_context_t *tsp_ctx, uint64_t ret)
{
	assert(tsp_ctx != NULL);
	/* Save the Secure EL1 system register context */
	assert(cm_get_context(SECURE) == &tsp_ctx->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);

	assert(tsp_ctx->c_rt_ctx != 0);
	tspd_exit_sp(tsp_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}

/*******************************************************************************
 * This function takes an SP context pointer and abort any preempted SMC
 * request.
 * Return 1 if there was a preempted SMC request, 0 otherwise.
 ******************************************************************************/
int tspd_abort_preempted_smc(tsp_context_t *tsp_ctx)
{
	if (!get_yield_smc_active_flag(tsp_ctx->state))
		return 0;

	/* Abort any preempted SMC request */
	clr_yield_smc_active_flag(tsp_ctx->state);

	/*
	 * Arrange for an entry into the test secure payload. It will
	 * be returned via TSP_ABORT_DONE case in tspd_smc_handler.
	 */
	cm_set_elr_el3(SECURE,
		       (uint64_t) &tsp_vectors->abort_yield_smc_entry);
	uint64_t rc = tspd_synchronous_sp_entry(tsp_ctx);

	if (rc != 0)
		panic();

	return 1;
}

