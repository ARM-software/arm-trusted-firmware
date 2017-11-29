/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <amu.h>
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
#include <utils.h>

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
	zeromem(ctx, sizeof(*ctx));

	reg_ctx = get_regs_ctx(ctx);

	/*
	 * Base the context SCR on the current value, adjust for entry point
	 * specific requirements
	 */
	scr = read_scr();
	scr &= ~(SCR_NS_BIT | SCR_HCE_BIT);

	if (security_state != SECURE)
		scr |= SCR_NS_BIT;

	if (security_state != SECURE) {
		/*
		 * Set up SCTLR for the Non-secure context.
		 *
		 * SCTLR.EE: Endianness is taken from the entrypoint attributes.
		 *
		 * SCTLR.M, SCTLR.C and SCTLR.I: These fields must be zero (as
		 *  required by PSCI specification)
		 *
		 * Set remaining SCTLR fields to their architecturally defined
		 * values. Some fields reset to an IMPLEMENTATION DEFINED value:
		 *
		 * SCTLR.TE: Set to zero so that exceptions to an Exception
		 *  Level executing at PL1 are taken to A32 state.
		 *
		 * SCTLR.V: Set to zero to select the normal exception vectors
		 *  with base address held in VBAR.
		 */
		assert(((ep->spsr >> SPSR_E_SHIFT) & SPSR_E_MASK) ==
			(EP_GET_EE(ep->h.attr) >> EP_EE_SHIFT));

		sctlr = EP_GET_EE(ep->h.attr) ? SCTLR_EE_BIT : 0;
		sctlr |= (SCTLR_RESET_VAL & ~(SCTLR_TE_BIT | SCTLR_V_BIT));
		write_ctx_reg(reg_ctx, CTX_NS_SCTLR, sctlr);
	}

	/*
	 * The target exception level is based on the spsr mode requested. If
	 * execution is requested to hyp mode, HVC is enabled via SCR.HCE.
	 */
	if (GET_M32(ep->spsr) == MODE32_hyp)
		scr |= SCR_HCE_BIT;

	/*
	 * Store the initialised values for SCTLR and SCR in the cpu_context.
	 * The Hyp mode registers are not part of the saved context and are
	 * set-up in cm_prepare_el3_exit().
	 */
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
 * Enable architecture extensions on first entry to Non-secure world.
 * When EL2 is implemented but unused `el2_unused` is non-zero, otherwise
 * it is zero.
 ******************************************************************************/
static void enable_extensions_nonsecure(int el2_unused)
{
#if IMAGE_BL32
#if ENABLE_AMU
	amu_enable(el2_unused);
#endif
#endif
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
	uint32_t hsctlr, scr;
	cpu_context_t *ctx = cm_get_context(security_state);
	int el2_unused = 0;

	assert(ctx);

	if (security_state == NON_SECURE) {
		scr = read_ctx_reg(get_regs_ctx(ctx), CTX_SCR);
		if (scr & SCR_HCE_BIT) {
			/* Use SCTLR value to initialize HSCTLR */
			hsctlr = read_ctx_reg(get_regs_ctx(ctx),
						 CTX_NS_SCTLR);
			hsctlr |= HSCTLR_RES1;
			/* Temporarily set the NS bit to access HSCTLR */
			write_scr(read_scr() | SCR_NS_BIT);
			/*
			 * Make sure the write to SCR is complete so that
			 * we can access HSCTLR
			 */
			isb();
			write_hsctlr(hsctlr);
			isb();

			write_scr(read_scr() & ~SCR_NS_BIT);
			isb();
		} else if (read_id_pfr1() &
			(ID_PFR1_VIRTEXT_MASK << ID_PFR1_VIRTEXT_SHIFT)) {
			el2_unused = 1;

			/*
			 * Set the NS bit to access NS copies of certain banked
			 * registers
			 */
			write_scr(read_scr() | SCR_NS_BIT);
			isb();

			/*
			 * Hyp / PL2 present but unused, need to disable safely.
			 * HSCTLR can be ignored in this case.
			 *
			 * Set HCR to its architectural reset value so that
			 * Non-secure operations do not trap to Hyp mode.
			 */
			write_hcr(HCR_RESET_VAL);

			/*
			 * Set HCPTR to its architectural reset value so that
			 * Non-secure access from EL1 or EL0 to trace and to
			 * Advanced SIMD and floating point functionality does
			 * not trap to Hyp mode.
			 */
			write_hcptr(HCPTR_RESET_VAL);

			/*
			 * Initialise CNTHCTL. All fields are architecturally
			 * UNKNOWN on reset and are set to zero except for
			 * field(s) listed below.
			 *
			 * CNTHCTL.PL1PCEN: Disable traps to Hyp mode of
			 *  Non-secure EL0 and EL1 accessed to the physical
			 *  timer registers.
			 *
			 * CNTHCTL.PL1PCTEN: Disable traps to Hyp mode of
			 *  Non-secure EL0 and EL1 accessed to the physical
			 *  counter registers.
			 */
			write_cnthctl(CNTHCTL_RESET_VAL |
					PL1PCEN_BIT | PL1PCTEN_BIT);

			/*
			 * Initialise CNTVOFF to zero as it resets to an
			 * IMPLEMENTATION DEFINED value.
			 */
			write64_cntvoff(0);

			/*
			 * Set VPIDR and VMPIDR to match MIDR_EL1 and MPIDR
			 * respectively.
			 */
			write_vpidr(read_midr());
			write_vmpidr(read_mpidr());

			/*
			 * Initialise VTTBR, setting all fields rather than
			 * relying on the hw. Some fields are architecturally
			 * UNKNOWN at reset.
			 *
			 * VTTBR.VMID: Set to zero which is the architecturally
			 *  defined reset value. Even though EL1&0 stage 2
			 *  address translation is disabled, cache maintenance
			 *  operations depend on the VMID.
			 *
			 * VTTBR.BADDR: Set to zero as EL1&0 stage 2 address
			 *  translation is disabled.
			 */
			write64_vttbr(VTTBR_RESET_VAL &
				~((VTTBR_VMID_MASK << VTTBR_VMID_SHIFT)
				| (VTTBR_BADDR_MASK << VTTBR_BADDR_SHIFT)));

			/*
			 * Initialise HDCR, setting all the fields rather than
			 * relying on hw.
			 *
			 * HDCR.HPMN: Set to value of PMCR.N which is the
			 *  architecturally-defined reset value.
			 */
			write_hdcr(HDCR_RESET_VAL |
				((read_pmcr() & PMCR_N_BITS) >> PMCR_N_SHIFT));

			/*
			 * Set HSTR to its architectural reset value so that
			 * access to system registers in the cproc=1111
			 * encoding space do not trap to Hyp mode.
			 */
			write_hstr(HSTR_RESET_VAL);
			/*
			 * Set CNTHP_CTL to its architectural reset value to
			 * disable the EL2 physical timer and prevent timer
			 * interrupts. Some fields are architecturally UNKNOWN
			 * on reset and are set to zero.
			 */
			write_cnthp_ctl(CNTHP_CTL_RESET_VAL);
			isb();

			write_scr(read_scr() & ~SCR_NS_BIT);
			isb();
		}
		enable_extensions_nonsecure(el2_unused);
	}
}
