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
#include "tspd_private.h"

/*******************************************************************************
 * Given a secure payload entrypoint, register width, cpu id & pointer to a
 * context data structure, this function will create a secure context ready for
 * programming an entry into the secure payload.
 ******************************************************************************/
int32_t tspd_init_secure_context(uint64_t entrypoint,
				 uint32_t rw,
				 uint64_t mpidr,
				 tsp_context_t *tsp_ctx)
{
	uint32_t scr, sctlr;
	el1_sys_regs_t *el1_state;
	uint32_t spsr;

	/* Passing a NULL context is a critical programming error */
	assert(tsp_ctx);

	/*
	 * We support AArch64 TSP for now.
	 * TODO: Add support for AArch32 TSP
	 */
	assert(rw == TSP_AARCH64);

	/*
	 * This might look redundant if the context was statically
	 * allocated but this function cannot make that assumption.
	 */
	memset(tsp_ctx, 0, sizeof(*tsp_ctx));

	/*
	 * Set the right security state, register width and enable access to
	 * the secure physical timer for the SP.
	 */
	scr = read_scr();
	scr &= ~SCR_NS_BIT;
	scr &= ~SCR_RW_BIT;
	scr |= SCR_ST_BIT;
	if (rw == TSP_AARCH64)
		scr |= SCR_RW_BIT;

	/* Get a pointer to the S-EL1 context memory */
	el1_state = get_sysregs_ctx(&tsp_ctx->cpu_ctx);

	/*
	 * Program the SCTLR_EL1 such that upon entry in S-EL1, caches and MMU are
	 * disabled and exception endianess is set to be the same as EL3
	 */
	sctlr = read_sctlr_el3();
	sctlr &= SCTLR_EE_BIT;
	sctlr |= SCTLR_EL1_RES1;
	write_ctx_reg(el1_state, CTX_SCTLR_EL1, sctlr);

	/* Set this context as ready to be initialised i.e OFF */
	set_tsp_pstate(tsp_ctx->state, TSP_PSTATE_OFF);

	/*
	 * This context has not been used yet. It will become valid
	 * when the TSP is interrupted and wants the TSPD to preserve
	 * the context.
	 */
	clr_std_smc_active_flag(tsp_ctx->state);

	/* Associate this context with the cpu specified */
	tsp_ctx->mpidr = mpidr;

	cm_set_context(mpidr, &tsp_ctx->cpu_ctx, SECURE);
	spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	cm_set_el3_eret_context(SECURE, entrypoint, spsr, scr);

	return 0;
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

	assert(tsp_ctx->c_rt_ctx == 0);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context(read_mpidr(), SECURE) == &tsp_ctx->cpu_ctx);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = tspd_enter_sp(&tsp_ctx->c_rt_ctx);
#if DEBUG
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
	/* Save the Secure EL1 system register context */
	assert(cm_get_context(read_mpidr(), SECURE) == &tsp_ctx->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);

	assert(tsp_ctx->c_rt_ctx != 0);
	tspd_exit_sp(tsp_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}
