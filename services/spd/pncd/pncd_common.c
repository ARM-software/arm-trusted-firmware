/*
 * Copyright (c) 2021-2022, ProvenRun S.A.S. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include "pncd_private.h"

/*******************************************************************************
 * Given a secure payload entrypoint info pointer, entry point PC & pointer to a
 * context data structure, this function will initialize pnc context and entry
 * point info for the secure payload
 ******************************************************************************/
void pncd_init_pnc_ep_state(struct entry_point_info *pnc_entry_point,
				uint64_t pc,
				pnc_context_t *pnc_ctx)
{
	uint32_t ep_attr;

	/* Passing a NULL context is a critical programming error */
	assert(pnc_ctx);
	assert(pnc_entry_point);
	assert(pc);

	/* Associate this context with the current cpu */
	pnc_ctx->mpidr = read_mpidr();

	cm_set_context(&pnc_ctx->cpu_ctx, SECURE);

	/* initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT) {
		ep_attr |= EP_EE_BIG;
	}
	SET_PARAM_HEAD(pnc_entry_point, PARAM_EP, VERSION_1, ep_attr);

	pnc_entry_point->pc = pc;
	pnc_entry_point->spsr = SPSR_64(MODE_EL1,
					MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS);
	memset(&pnc_entry_point->args, 0, sizeof(pnc_entry_point->args));
}

/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Applies the S-EL1 system register context from pnc_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the pnc_ctx->cpu_ctx are used to enter the secure payload image.
 ******************************************************************************/
uint64_t pncd_synchronous_sp_entry(pnc_context_t *pnc_ctx)
{
	assert(pnc_ctx != NULL);
	assert(pnc_ctx->c_rt_ctx == 0U);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context(SECURE) == &pnc_ctx->cpu_ctx);
	cm_el1_sysregs_context_restore(SECURE);
#if CTX_INCLUDE_FPREGS
	fpregs_context_restore(get_fpregs_ctx(cm_get_context(SECURE)));
#endif
	cm_set_next_eret_context(SECURE);

	return pncd_enter_sp(&pnc_ctx->c_rt_ctx);
}


/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Saves the S-EL1 system register context tp pnc_ctx->cpu_ctx.
 * 2. Restores the current C runtime state (callee saved registers) from the
 *    stack frame using the reference to this state saved in pncd_enter_sp().
 * 3. It does not need to save any general purpose or EL3 system register state
 *    as the generic smc entry routine should have saved those.
 ******************************************************************************/
void pncd_synchronous_sp_exit(pnc_context_t *pnc_ctx, uint64_t ret)
{
	assert(pnc_ctx != NULL);
	/* Save the Secure EL1 system register context */
	assert(cm_get_context(SECURE) == &pnc_ctx->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);
#if CTX_INCLUDE_FPREGS
	fpregs_context_save(get_fpregs_ctx(cm_get_context(SECURE)));
#endif

	assert(pnc_ctx->c_rt_ctx != 0);
	pncd_exit_sp(pnc_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	panic();
}
