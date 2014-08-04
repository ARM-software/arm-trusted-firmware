/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <string.h>
#include "opteed_private.h"

/*******************************************************************************
 * Given a OPTEE entrypoint info pointer, entry point PC, register width,
 * cpu id & pointer to a context data structure, this function will
 * initialize OPTEE context and entry point info for OPTEE.
 ******************************************************************************/
void opteed_init_optee_ep_state(struct entry_point_info *optee_entry_point,
				uint32_t rw, uint64_t pc,
				optee_context_t *optee_ctx)
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
	memset(&optee_entry_point->args, 0, sizeof(optee_entry_point->args));
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
#if DEBUG
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
