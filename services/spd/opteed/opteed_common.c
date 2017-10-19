/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <string.h>
#include <utils.h>
#include "opteed_private.h"

/*******************************************************************************
 * Given a OPTEE entrypoint info pointer, entry point PC, register width,
 * cpu id & pointer to a context data structure, this function will
 * initialize OPTEE context and entry point info for OPTEE.
 ******************************************************************************/
void opteed_init_optee_ep_state(struct entry_point_info *optee_entry_point,
				uint32_t rw, uint64_t pc,
				uint64_t pageable_part, uint64_t mem_limit,
				uint64_t dt_addr, optee_context_t *optee_ctx)
{
	uint32_t ep_attr;

	/* Passing a NULL context is a critical programming error */
	assert(optee_ctx);
	assert(optee_entry_point);
	assert(pc);

	/* Associate this context with the cpu specified */
	optee_ctx->mpidr = read_mpidr_el1();
	optee_ctx->state = 0;
	set_optee_pstate(optee_ctx->state, OPTEE_PSTATE_OFF);

	cm_set_context(&optee_ctx->cpu_ctx, SECURE);

	/* initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT)
		ep_attr |= EP_EE_BIG;
	SET_PARAM_HEAD(optee_entry_point, PARAM_EP, VERSION_1, ep_attr);
	optee_entry_point->pc = pc;
	if (rw == OPTEE_AARCH64)
		optee_entry_point->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
						  DISABLE_ALL_EXCEPTIONS);
	else
		optee_entry_point->spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
						      SPSR_E_LITTLE,
						      DAIF_FIQ_BIT |
							DAIF_IRQ_BIT |
							DAIF_ABT_BIT);
	zeromem(&optee_entry_point->args, sizeof(optee_entry_point->args));
	optee_entry_point->args.arg0 = pageable_part;
	optee_entry_point->args.arg1 = mem_limit;
	optee_entry_point->args.arg2 = dt_addr;
}

/*******************************************************************************
 * This function takes an OPTEE context pointer and:
 * 1. Applies the S-EL1 system register context from optee_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the optee_ctx->cpu_ctx are used to enter the OPTEE image.
 ******************************************************************************/
uint64_t opteed_synchronous_sp_entry(optee_context_t *optee_ctx)
{
	uint64_t rc;

	assert(optee_ctx != NULL);
	assert(optee_ctx->c_rt_ctx == 0);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context(SECURE) == &optee_ctx->cpu_ctx);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = opteed_enter_sp(&optee_ctx->c_rt_ctx);
#if ENABLE_ASSERTIONS
	optee_ctx->c_rt_ctx = 0;
#endif

	return rc;
}


/*******************************************************************************
 * This function takes an OPTEE context pointer and:
 * 1. Saves the S-EL1 system register context tp optee_ctx->cpu_ctx.
 * 2. Restores the current C runtime state (callee saved registers) from the
 *    stack frame using the reference to this state saved in opteed_enter_sp().
 * 3. It does not need to save any general purpose or EL3 system register state
 *    as the generic smc entry routine should have saved those.
 ******************************************************************************/
void opteed_synchronous_sp_exit(optee_context_t *optee_ctx, uint64_t ret)
{
	assert(optee_ctx != NULL);
	/* Save the Secure EL1 system register context */
	assert(cm_get_context(SECURE) == &optee_ctx->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);

	assert(optee_ctx->c_rt_ctx != 0);
	opteed_exit_sp(optee_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}
