/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved
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
#include "tlkd_private.h"

/*******************************************************************************
 * Function to get Physical address after a V->P translation
 ******************************************************************************/
DEFINE_SYSREG_READ_FUNC(par_el1)

/*******************************************************************************
 * This function helps the SP to translate NS/S virtual addresses.
 ******************************************************************************/
uint64_t tlkd_va_translate(uintptr_t va, int type)
{
	uint32_t scr;
	uint64_t pa;

	if (type & TLK_TRANSLATE_NS_VADDR) {

		/* save secure context */
		cm_el1_sysregs_context_save(SECURE);

		/* restore non-secure context */
		cm_el1_sysregs_context_restore(NON_SECURE);

		/* switch NS bit to start using 64-bit, non-secure mappings */
		scr = read_scr();
		write_scr(scr | SCR_NS_BIT | SCR_RW_BIT);
		isb();
	}

	int at = type & 3;
	switch (at) {
	case 0:
		tlkd_priv_read(va);
		break;
	case 1:
		tlkd_priv_write(va);
		break;
	case 2:
		tlkd_user_read(va);
		break;
	case 3:
		tlkd_user_write(va);
		break;
	default:
		assert(0);
	}

	/* get the (NS/S) physical address */
	pa = read_par_el1();

	/* Restore secure state */
	if (type & TLK_TRANSLATE_NS_VADDR) {

		/* restore secure context */
		cm_el1_sysregs_context_restore(SECURE);

		/* switch NS bit to start using 32-bit, secure mappings */
		write_scr(scr);
		isb();
	}

	return pa;
}

/*******************************************************************************
 * Given a secure payload entrypoint, register width, cpu id & pointer to a
 * context data structure, this function will create a secure context ready for
 * programming an entry into the secure payload.
 ******************************************************************************/
void tlkd_init_tlk_ep_state(struct entry_point_info *tlk_entry_point,
			    uint32_t rw,
			    uint64_t pc,
			    tlk_context_t *tlk_ctx)
{
	uint32_t ep_attr, spsr;

	/* Passing a NULL context is a critical programming error */
	assert(tlk_ctx);
	assert(tlk_entry_point);
	assert(pc);

	/* Associate this context with the cpu specified */
	tlk_ctx->mpidr = read_mpidr_el1();
	clr_std_smc_active_flag(tlk_ctx->state);
	cm_set_context(&tlk_ctx->cpu_ctx, SECURE);

	if (rw == SP_AARCH64)
		spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	else
		spsr = SPSR_MODE32(MODE32_svc,
				   SPSR_T_ARM,
				   read_sctlr_el3() & SCTLR_EE_BIT,
				   DISABLE_ALL_EXCEPTIONS);

	/* initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT)
		ep_attr |= EP_EE_BIG;
	SET_PARAM_HEAD(tlk_entry_point, PARAM_EP, VERSION_1, ep_attr);

	tlk_entry_point->pc = pc;
	tlk_entry_point->spsr = spsr;
}

/*******************************************************************************
 * This function takes a TLK context pointer and:
 * 1. Applies the S-EL1 system register context from tlk_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the tlk_ctx->cpu_ctx are used to enter the secure payload image.
 ******************************************************************************/
uint64_t tlkd_synchronous_sp_entry(tlk_context_t *tlk_ctx)
{
	uint64_t rc;

	/* Passing a NULL context is a critical programming error */
	assert(tlk_ctx);
	assert(tlk_ctx->c_rt_ctx == 0);

	/* Apply the Secure EL1 system register context and switch to it */
	assert(cm_get_context(SECURE) == &tlk_ctx->cpu_ctx);
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	rc = tlkd_enter_sp(&tlk_ctx->c_rt_ctx);
#if DEBUG
	tlk_ctx->c_rt_ctx = 0;
#endif

	return rc;
}

/*******************************************************************************
 * This function takes a TLK context pointer and:
 * 1. Saves the S-EL1 system register context to tlk_ctx->cpu_ctx.
 * 2. Restores the current C runtime state (callee saved registers) from the
 *    stack frame using reference to this state saved in tlkd_enter_sp().
 * 3. It does not need to save any general purpose or EL3 system register state
 *    as the generic smc entry routine should have saved those.
 ******************************************************************************/
void tlkd_synchronous_sp_exit(tlk_context_t *tlk_ctx, uint64_t ret)
{
	/* Passing a NULL context is a critical programming error */
	assert(tlk_ctx);

	/* Save the Secure EL1 system register context */
	assert(cm_get_context(SECURE) == &tlk_ctx->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);

	assert(tlk_ctx->c_rt_ctx != 0);
	tlkd_exit_sp(tlk_ctx->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}
