/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <platform.h>
#include <platform_def.h>
#include <smcc_helpers.h>
#include <string.h>

/*******************************************************************************
 * Context management library initialisation routine. This library is used by
 * runtime services to share pointers to 'cpu_context' structures for the secure
 * and non-secure states. Management of the structures and their associated
 * memory is not done by the context management library e.g. the PSCI service
 * manages the cpu context used for entry from and exit to the non-secure state.
 * The Secure payload manages the context(s) corresponding to the secure state.
 * It also uses this library to get access to the non-secure
 * state cpu context pointers.
 ******************************************************************************/
void cm_init(void)
{
	/*
	 * The context management library has only global data to initialize, but
	 * that will be done when the BSS is zeroed out
	 */
}

/*******************************************************************************
 * The following function initializes the cpu_context 'ctx' for
 * first use, and sets the initial entrypoint state as specified by the
 * entry_point_info structure.
 *
 * The security state to initialize is determined by the SECURE attribute
 * of the entry_point_info. The function returns a pointer to the initialized
 * context and sets this as the next context to return to.
 *
 * The EE and ST attributes are used to configure the endianness and secure
 * timer availability for the new execution context.
 *
 * To prepare the register state for entry call cm_prepare_el3_exit() and
 * el3_exit(). For Secure-EL1 cm_prepare_el3_exit() is equivalent to
 * cm_e1_sysreg_context_restore().
 ******************************************************************************/
static void cm_init_context_common(cpu_context_t *ctx, const entry_point_info_t *ep)
{
	unsigned int security_state;
	uint32_t scr, sctlr;
	regs_t *reg_ctx;

	assert(ctx);

	security_state = GET_SECURITY_STATE(ep->h.attr);

	/* Clear any residual register values from the context */
	memset(ctx, 0, sizeof(*ctx));

	reg_ctx = get_regs_ctx(ctx);

	/*
	 * Base the context SCR on the current value, adjust for entry point
	 * specific requirements
	 */
	scr = read_scr();
	scr &= ~(SCR_NS_BIT | SCR_HCE_BIT);

	if (security_state != SECURE)
		scr |= SCR_NS_BIT;

	/*
	 * Set up SCTLR for the Non Secure context.
	 * EE bit is taken from the entrypoint attributes
	 * M, C and I bits must be zero (as required by PSCI specification)
	 *
	 * The target exception level is based on the spsr mode requested.
	 * If execution is requested to hyp mode, HVC is enabled
	 * via SCR.HCE.
	 *
	 * Always compute the SCTLR_EL1 value and save in the cpu_context
	 * - the HYP registers are set up by cm_preapre_ns_entry() as they
	 * are not part of the stored cpu_context
	 *
	 * TODO: In debug builds the spsr should be validated and checked
	 * against the CPU support, security state, endianness and pc
	 */
	if (security_state != SECURE) {
		sctlr = EP_GET_EE(ep->h.attr) ? SCTLR_EE_BIT : 0;
		sctlr |= SCTLR_RES1;
		write_ctx_reg(reg_ctx, CTX_NS_SCTLR, sctlr);
	}

	if (GET_M32(ep->spsr) == MODE32_hyp)
		scr |= SCR_HCE_BIT;

	write_ctx_reg(reg_ctx, CTX_SCR, scr);
	write_ctx_reg(reg_ctx, CTX_LR, ep->pc);
	write_ctx_reg(reg_ctx, CTX_SPSR, ep->spsr);

	/*
	 * Store the r0-r3 value from the entrypoint into the context
	 * Use memcpy as we are in control of the layout of the structures
	 */
	memcpy((void *)reg_ctx, (void *)&ep->args, sizeof(aapcs32_params_t));
}

/*******************************************************************************
 * The following function initializes the cpu_context for a CPU specified by
 * its `cpu_idx` for first use, and sets the initial entrypoint state as
 * specified by the entry_point_info structure.
 ******************************************************************************/
void cm_init_context_by_index(unsigned int cpu_idx,
			      const entry_point_info_t *ep)
{
	cpu_context_t *ctx;
	ctx = cm_get_context_by_index(cpu_idx, GET_SECURITY_STATE(ep->h.attr));
	cm_init_context_common(ctx, ep);
}

/*******************************************************************************
 * The following function initializes the cpu_context for the current CPU
 * for first use, and sets the initial entrypoint state as specified by the
 * entry_point_info structure.
 ******************************************************************************/
void cm_init_my_context(const entry_point_info_t *ep)
{
	cpu_context_t *ctx;
	ctx = cm_get_context(GET_SECURITY_STATE(ep->h.attr));
	cm_init_context_common(ctx, ep);
}

/*******************************************************************************
 * Prepare the CPU system registers for first entry into secure or normal world
 *
 * If execution is requested to hyp mode, HSCTLR is initialized
 * If execution is requested to non-secure PL1, and the CPU supports
 * HYP mode then HYP mode is disabled by configuring all necessary HYP mode
 * registers.
 ******************************************************************************/
void cm_prepare_el3_exit(uint32_t security_state)
{
	uint32_t sctlr, scr, hcptr;
	cpu_context_t *ctx = cm_get_context(security_state);

	assert(ctx);

	if (security_state == NON_SECURE) {
		scr = read_ctx_reg(get_regs_ctx(ctx), CTX_SCR);
		if (scr & SCR_HCE_BIT) {
			/* Use SCTLR value to initialize HSCTLR */
			sctlr = read_ctx_reg(get_regs_ctx(ctx),
						 CTX_NS_SCTLR);
			sctlr |= HSCTLR_RES1;
			/* Temporarily set the NS bit to access HSCTLR */
			write_scr(read_scr() | SCR_NS_BIT);
			/*
			 * Make sure the write to SCR is complete so that
			 * we can access HSCTLR
			 */
			isb();
			write_hsctlr(sctlr);
			isb();

			write_scr(read_scr() & ~SCR_NS_BIT);
			isb();
		} else if (read_id_pfr1() &
			(ID_PFR1_VIRTEXT_MASK << ID_PFR1_VIRTEXT_SHIFT)) {
			/* Set the NS bit to access HCR, HCPTR, CNTHCTL, VPIDR, VMPIDR */
			write_scr(read_scr() | SCR_NS_BIT);
			isb();

			/* PL2 present but unused, need to disable safely */
			write_hcr(0);

			/* HSCTLR : can be ignored when bypassing */

			/* HCPTR : disable all traps TCPAC, TTA, TCP */
			hcptr = read_hcptr();
			hcptr &= ~(TCPAC_BIT | TTA_BIT | TCP11_BIT | TCP10_BIT);
			write_hcptr(hcptr);

			/* Enable EL1 access to timer */
			write_cnthctl(PL1PCEN_BIT | PL1PCTEN_BIT);

			/* Reset CNTVOFF_EL2 */
			write64_cntvoff(0);

			/* Set VPIDR, VMPIDR to match MIDR, MPIDR */
			write_vpidr(read_midr());
			write_vmpidr(read_mpidr());

			/*
			 * Reset VTTBR.
			 * Needed because cache maintenance operations depend on
			 * the VMID even when non-secure EL1&0 stage 2 address
			 * translation are disabled.
			 */
			write64_vttbr(0);
			isb();

			write_scr(read_scr() & ~SCR_NS_BIT);
			isb();
		}
	}
}
