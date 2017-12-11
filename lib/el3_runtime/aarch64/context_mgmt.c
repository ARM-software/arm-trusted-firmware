/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
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
#include <interrupt_mgmt.h>
#include <platform.h>
#include <platform_def.h>
#include <pubsub_events.h>
#include <smcc_helpers.h>
#include <spe.h>
#include <string.h>
#include <sve.h>
#include <utils.h>


/*******************************************************************************
 * Context management library initialisation routine. This library is used by
 * runtime services to share pointers to 'cpu_context' structures for the secure
 * and non-secure states. Management of the structures and their associated
 * memory is not done by the context management library e.g. the PSCI service
 * manages the cpu context used for entry from and exit to the non-secure state.
 * The Secure payload dispatcher service manages the context(s) corresponding to
 * the secure state. It also uses this library to get access to the non-secure
 * state cpu context pointers.
 * Lastly, this library provides the api to make SP_EL3 point to the cpu context
 * which will used for programming an entry into a lower EL. The same context
 * will used to save state upon exception entry from that EL.
 ******************************************************************************/
void cm_init(void)
{
	/*
	 * The context management library has only global data to intialize, but
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
 * The EE and ST attributes are used to configure the endianess and secure
 * timer availability for the new execution context.
 *
 * To prepare the register state for entry call cm_prepare_el3_exit() and
 * el3_exit(). For Secure-EL1 cm_prepare_el3_exit() is equivalent to
 * cm_e1_sysreg_context_restore().
 ******************************************************************************/
static void cm_init_context_common(cpu_context_t *ctx, const entry_point_info_t *ep)
{
	unsigned int security_state;
	uint32_t scr_el3, pmcr_el0;
	el3_state_t *state;
	gp_regs_t *gp_regs;
	unsigned long sctlr_elx;

	assert(ctx);

	security_state = GET_SECURITY_STATE(ep->h.attr);

	/* Clear any residual register values from the context */
	zeromem(ctx, sizeof(*ctx));

	/*
	 * SCR_EL3 was initialised during reset sequence in macro
	 * el3_arch_init_common. This code modifies the SCR_EL3 fields that
	 * affect the next EL.
	 *
	 * The following fields are initially set to zero and then updated to
	 * the required value depending on the state of the SPSR_EL3 and the
	 * Security state and entrypoint attributes of the next EL.
	 */
	scr_el3 = read_scr();
	scr_el3 &= ~(SCR_NS_BIT | SCR_RW_BIT | SCR_FIQ_BIT | SCR_IRQ_BIT |
			SCR_ST_BIT | SCR_HCE_BIT);
	/*
	 * SCR_NS: Set the security state of the next EL.
	 */
	if (security_state != SECURE)
		scr_el3 |= SCR_NS_BIT;
	/*
	 * SCR_EL3.RW: Set the execution state, AArch32 or AArch64, for next
	 *  Exception level as specified by SPSR.
	 */
	if (GET_RW(ep->spsr) == MODE_RW_64)
		scr_el3 |= SCR_RW_BIT;
	/*
	 * SCR_EL3.ST: Traps Secure EL1 accesses to the Counter-timer Physical
	 *  Secure timer registers to EL3, from AArch64 state only, if specified
	 *  by the entrypoint attributes.
	 */
	if (EP_GET_ST(ep->h.attr))
		scr_el3 |= SCR_ST_BIT;

#ifndef HANDLE_EA_EL3_FIRST
	/*
	 * SCR_EL3.EA: Do not route External Abort and SError Interrupt External
	 *  to EL3 when executing at a lower EL. When executing at EL3, External
	 *  Aborts are taken to EL3.
	 */
	scr_el3 &= ~SCR_EA_BIT;
#endif

#ifdef IMAGE_BL31
	/*
	 * SCR_EL3.IRQ, SCR_EL3.FIQ: Enable the physical FIQ and IRQ rounting as
	 *  indicated by the interrupt routing model for BL31.
	 */
	scr_el3 |= get_scr_el3_from_routing_model(security_state);
#endif

	/*
	 * SCR_EL3.HCE: Enable HVC instructions if next execution state is
	 * AArch64 and next EL is EL2, or if next execution state is AArch32 and
	 * next mode is Hyp.
	 */
	if ((GET_RW(ep->spsr) == MODE_RW_64
	     && GET_EL(ep->spsr) == MODE_EL2)
	    || (GET_RW(ep->spsr) != MODE_RW_64
		&& GET_M32(ep->spsr) == MODE32_hyp)) {
		scr_el3 |= SCR_HCE_BIT;
	}

	/*
	 * Initialise SCTLR_EL1 to the reset value corresponding to the target
	 * execution state setting all fields rather than relying of the hw.
	 * Some fields have architecturally UNKNOWN reset values and these are
	 * set to zero.
	 *
	 * SCTLR.EE: Endianness is taken from the entrypoint attributes.
	 *
	 * SCTLR.M, SCTLR.C and SCTLR.I: These fields must be zero (as
	 *  required by PSCI specification)
	 */
	sctlr_elx = EP_GET_EE(ep->h.attr) ? SCTLR_EE_BIT : 0;
	if (GET_RW(ep->spsr) == MODE_RW_64)
		sctlr_elx |= SCTLR_EL1_RES1;
	else {
		/*
		 * If the target execution state is AArch32 then the following
		 * fields need to be set.
		 *
		 * SCTRL_EL1.nTWE: Set to one so that EL0 execution of WFE
		 *  instructions are not trapped to EL1.
		 *
		 * SCTLR_EL1.nTWI: Set to one so that EL0 execution of WFI
		 *  instructions are not trapped to EL1.
		 *
		 * SCTLR_EL1.CP15BEN: Set to one to enable EL0 execution of the
		 *  CP15DMB, CP15DSB, and CP15ISB instructions.
		 */
		sctlr_elx |= SCTLR_AARCH32_EL1_RES1 | SCTLR_CP15BEN_BIT
					| SCTLR_NTWI_BIT | SCTLR_NTWE_BIT;
	}

	/*
	 * Store the initialised SCTLR_EL1 value in the cpu_context - SCTLR_EL2
	 * and other EL2 registers are set up by cm_preapre_ns_entry() as they
	 * are not part of the stored cpu_context.
	 */
	write_ctx_reg(get_sysregs_ctx(ctx), CTX_SCTLR_EL1, sctlr_elx);

	if (security_state == SECURE) {
		/*
		 * Initialise PMCR_EL0 for secure context only, setting all
		 * fields rather than relying on hw. Some fields are
		 * architecturally UNKNOWN on reset.
		 *
		 * PMCR_EL0.LC: Set to one so that cycle counter overflow, that
		 *  is recorded in PMOVSCLR_EL0[31], occurs on the increment
		 *  that changes PMCCNTR_EL0[63] from 1 to 0.
		 *
		 * PMCR_EL0.DP: Set to one so that the cycle counter,
		 *  PMCCNTR_EL0 does not count when event counting is prohibited.
		 *
		 * PMCR_EL0.X: Set to zero to disable export of events.
		 *
		 * PMCR_EL0.D: Set to zero so that, when enabled, PMCCNTR_EL0
		 *  counts on every clock cycle.
		 */
		pmcr_el0 = ((PMCR_EL0_RESET_VAL | PMCR_EL0_LC_BIT
				| PMCR_EL0_DP_BIT)
				& ~(PMCR_EL0_X_BIT | PMCR_EL0_D_BIT));
		write_ctx_reg(get_sysregs_ctx(ctx), CTX_PMCR_EL0, pmcr_el0);
	}

	/* Populate EL3 state so that we've the right context before doing ERET */
	state = get_el3state_ctx(ctx);
	write_ctx_reg(state, CTX_SCR_EL3, scr_el3);
	write_ctx_reg(state, CTX_ELR_EL3, ep->pc);
	write_ctx_reg(state, CTX_SPSR_EL3, ep->spsr);

	/*
	 * Store the X0-X7 value from the entrypoint into the context
	 * Use memcpy as we are in control of the layout of the structures
	 */
	gp_regs = get_gpregs_ctx(ctx);
	memcpy(gp_regs, (void *)&ep->args, sizeof(aapcs64_params_t));
}

/*******************************************************************************
 * Enable architecture extensions on first entry to Non-secure world.
 * When EL2 is implemented but unused `el2_unused` is non-zero, otherwise
 * it is zero.
 ******************************************************************************/
static void enable_extensions_nonsecure(int el2_unused)
{
#if IMAGE_BL31
#if ENABLE_SPE_FOR_LOWER_ELS
	spe_enable(el2_unused);
#endif

#if ENABLE_AMU
	amu_enable(el2_unused);
#endif

#if ENABLE_SVE_FOR_NS
	sve_enable(el2_unused);
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
 * If execution is requested to EL2 or hyp mode, SCTLR_EL2 is initialized
 * If execution is requested to non-secure EL1 or svc mode, and the CPU supports
 * EL2 then EL2 is disabled by configuring all necessary EL2 registers.
 * For all entries, the EL1 registers are initialized from the cpu_context
 ******************************************************************************/
void cm_prepare_el3_exit(uint32_t security_state)
{
	uint32_t sctlr_elx, scr_el3, mdcr_el2;
	cpu_context_t *ctx = cm_get_context(security_state);
	int el2_unused = 0;

	assert(ctx);

	if (security_state == NON_SECURE) {
		scr_el3 = read_ctx_reg(get_el3state_ctx(ctx), CTX_SCR_EL3);
		if (scr_el3 & SCR_HCE_BIT) {
			/* Use SCTLR_EL1.EE value to initialise sctlr_el2 */
			sctlr_elx = read_ctx_reg(get_sysregs_ctx(ctx),
						 CTX_SCTLR_EL1);
			sctlr_elx &= SCTLR_EE_BIT;
			sctlr_elx |= SCTLR_EL2_RES1;
			write_sctlr_el2(sctlr_elx);
		} else if (EL_IMPLEMENTED(2)) {
			el2_unused = 1;

			/*
			 * EL2 present but unused, need to disable safely.
			 * SCTLR_EL2 can be ignored in this case.
			 *
			 * Initialise all fields in HCR_EL2, except HCR_EL2.RW,
			 * to zero so that Non-secure operations do not trap to
			 * EL2.
			 *
			 * HCR_EL2.RW: Set this field to match SCR_EL3.RW
			 */
			write_hcr_el2((scr_el3 & SCR_RW_BIT) ? HCR_RW_BIT : 0);

			/*
			 * Initialise CPTR_EL2 setting all fields rather than
			 * relying on the hw. All fields have architecturally
			 * UNKNOWN reset values.
			 *
			 * CPTR_EL2.TCPAC: Set to zero so that Non-secure EL1
			 *  accesses to the CPACR_EL1 or CPACR from both
			 *  Execution states do not trap to EL2.
			 *
			 * CPTR_EL2.TTA: Set to zero so that Non-secure System
			 *  register accesses to the trace registers from both
			 *  Execution states do not trap to EL2.
			 *
			 * CPTR_EL2.TFP: Set to zero so that Non-secure accesses
			 *  to SIMD and floating-point functionality from both
			 *  Execution states do not trap to EL2.
			 */
			write_cptr_el2(CPTR_EL2_RESET_VAL &
					~(CPTR_EL2_TCPAC_BIT | CPTR_EL2_TTA_BIT
					| CPTR_EL2_TFP_BIT));

			/*
			 * Initiliase CNTHCTL_EL2. All fields are
			 * architecturally UNKNOWN on reset and are set to zero
			 * except for field(s) listed below.
			 *
			 * CNTHCTL_EL2.EL1PCEN: Set to one to disable traps to
			 *  Hyp mode of Non-secure EL0 and EL1 accesses to the
			 *  physical timer registers.
			 *
			 * CNTHCTL_EL2.EL1PCTEN: Set to one to disable traps to
			 *  Hyp mode of  Non-secure EL0 and EL1 accesses to the
			 *  physical counter registers.
			 */
			write_cnthctl_el2(CNTHCTL_RESET_VAL |
						EL1PCEN_BIT | EL1PCTEN_BIT);

			/*
			 * Initialise CNTVOFF_EL2 to zero as it resets to an
			 * architecturally UNKNOWN value.
			 */
			write_cntvoff_el2(0);

			/*
			 * Set VPIDR_EL2 and VMPIDR_EL2 to match MIDR_EL1 and
			 * MPIDR_EL1 respectively.
			 */
			write_vpidr_el2(read_midr_el1());
			write_vmpidr_el2(read_mpidr_el1());

			/*
			 * Initialise VTTBR_EL2. All fields are architecturally
			 * UNKNOWN on reset.
			 *
			 * VTTBR_EL2.VMID: Set to zero. Even though EL1&0 stage
			 *  2 address translation is disabled, cache maintenance
			 *  operations depend on the VMID.
			 *
			 * VTTBR_EL2.BADDR: Set to zero as EL1&0 stage 2 address
			 *  translation is disabled.
			 */
			write_vttbr_el2(VTTBR_RESET_VAL &
				~((VTTBR_VMID_MASK << VTTBR_VMID_SHIFT)
				| (VTTBR_BADDR_MASK << VTTBR_BADDR_SHIFT)));

			/*
			 * Initialise MDCR_EL2, setting all fields rather than
			 * relying on hw. Some fields are architecturally
			 * UNKNOWN on reset.
			 *
			 * MDCR_EL2.TDRA: Set to zero so that Non-secure EL0 and
			 *  EL1 System register accesses to the Debug ROM
			 *  registers are not trapped to EL2.
			 *
			 * MDCR_EL2.TDOSA: Set to zero so that Non-secure EL1
			 *  System register accesses to the powerdown debug
			 *  registers are not trapped to EL2.
			 *
			 * MDCR_EL2.TDA: Set to zero so that System register
			 *  accesses to the debug registers do not trap to EL2.
			 *
			 * MDCR_EL2.TDE: Set to zero so that debug exceptions
			 *  are not routed to EL2.
			 *
			 * MDCR_EL2.HPME: Set to zero to disable EL2 Performance
			 *  Monitors.
			 *
			 * MDCR_EL2.TPM: Set to zero so that Non-secure EL0 and
			 *  EL1 accesses to all Performance Monitors registers
			 *  are not trapped to EL2.
			 *
			 * MDCR_EL2.TPMCR: Set to zero so that Non-secure EL0
			 *  and EL1 accesses to the PMCR_EL0 or PMCR are not
			 *  trapped to EL2.
			 *
			 * MDCR_EL2.HPMN: Set to value of PMCR_EL0.N which is the
			 *  architecturally-defined reset value.
			 */
			mdcr_el2 = ((MDCR_EL2_RESET_VAL |
					((read_pmcr_el0() & PMCR_EL0_N_BITS)
					>> PMCR_EL0_N_SHIFT)) &
					~(MDCR_EL2_TDRA_BIT | MDCR_EL2_TDOSA_BIT
					| MDCR_EL2_TDA_BIT | MDCR_EL2_TDE_BIT
					| MDCR_EL2_HPME_BIT | MDCR_EL2_TPM_BIT
					| MDCR_EL2_TPMCR_BIT));

			write_mdcr_el2(mdcr_el2);

			/*
			 * Initialise HSTR_EL2. All fields are architecturally
			 * UNKNOWN on reset.
			 *
			 * HSTR_EL2.T<n>: Set all these fields to zero so that
			 *  Non-secure EL0 or EL1 accesses to System registers
			 *  do not trap to EL2.
			 */
			write_hstr_el2(HSTR_EL2_RESET_VAL & ~(HSTR_EL2_T_MASK));
			/*
			 * Initialise CNTHP_CTL_EL2. All fields are
			 * architecturally UNKNOWN on reset.
			 *
			 * CNTHP_CTL_EL2:ENABLE: Set to zero to disable the EL2
			 *  physical timer and prevent timer interrupts.
			 */
			write_cnthp_ctl_el2(CNTHP_CTL_RESET_VAL &
						~(CNTHP_CTL_ENABLE_BIT));
		}
		enable_extensions_nonsecure(el2_unused);
	}

	cm_el1_sysregs_context_restore(security_state);
	cm_set_next_eret_context(security_state);
}

/*******************************************************************************
 * The next four functions are used by runtime services to save and restore
 * EL1 context on the 'cpu_context' structure for the specified security
 * state.
 ******************************************************************************/
void cm_el1_sysregs_context_save(uint32_t security_state)
{
	cpu_context_t *ctx;

	ctx = cm_get_context(security_state);
	assert(ctx);

	el1_sysregs_context_save(get_sysregs_ctx(ctx));

#if IMAGE_BL31
	if (security_state == SECURE)
		PUBLISH_EVENT(cm_exited_secure_world);
	else
		PUBLISH_EVENT(cm_exited_normal_world);
#endif
}

void cm_el1_sysregs_context_restore(uint32_t security_state)
{
	cpu_context_t *ctx;

	ctx = cm_get_context(security_state);
	assert(ctx);

	el1_sysregs_context_restore(get_sysregs_ctx(ctx));

#if IMAGE_BL31
	if (security_state == SECURE)
		PUBLISH_EVENT(cm_entering_secure_world);
	else
		PUBLISH_EVENT(cm_entering_normal_world);
#endif
}

/*******************************************************************************
 * This function populates ELR_EL3 member of 'cpu_context' pertaining to the
 * given security state with the given entrypoint
 ******************************************************************************/
void cm_set_elr_el3(uint32_t security_state, uintptr_t entrypoint)
{
	cpu_context_t *ctx;
	el3_state_t *state;

	ctx = cm_get_context(security_state);
	assert(ctx);

	/* Populate EL3 state so that ERET jumps to the correct entry */
	state = get_el3state_ctx(ctx);
	write_ctx_reg(state, CTX_ELR_EL3, entrypoint);
}

/*******************************************************************************
 * This function populates ELR_EL3 and SPSR_EL3 members of 'cpu_context'
 * pertaining to the given security state
 ******************************************************************************/
void cm_set_elr_spsr_el3(uint32_t security_state,
			uintptr_t entrypoint, uint32_t spsr)
{
	cpu_context_t *ctx;
	el3_state_t *state;

	ctx = cm_get_context(security_state);
	assert(ctx);

	/* Populate EL3 state so that ERET jumps to the correct entry */
	state = get_el3state_ctx(ctx);
	write_ctx_reg(state, CTX_ELR_EL3, entrypoint);
	write_ctx_reg(state, CTX_SPSR_EL3, spsr);
}

/*******************************************************************************
 * This function updates a single bit in the SCR_EL3 member of the 'cpu_context'
 * pertaining to the given security state using the value and bit position
 * specified in the parameters. It preserves all other bits.
 ******************************************************************************/
void cm_write_scr_el3_bit(uint32_t security_state,
			  uint32_t bit_pos,
			  uint32_t value)
{
	cpu_context_t *ctx;
	el3_state_t *state;
	uint32_t scr_el3;

	ctx = cm_get_context(security_state);
	assert(ctx);

	/* Ensure that the bit position is a valid one */
	assert((1 << bit_pos) & SCR_VALID_BIT_MASK);

	/* Ensure that the 'value' is only a bit wide */
	assert(value <= 1);

	/*
	 * Get the SCR_EL3 value from the cpu context, clear the desired bit
	 * and set it to its new value.
	 */
	state = get_el3state_ctx(ctx);
	scr_el3 = read_ctx_reg(state, CTX_SCR_EL3);
	scr_el3 &= ~(1 << bit_pos);
	scr_el3 |= value << bit_pos;
	write_ctx_reg(state, CTX_SCR_EL3, scr_el3);
}

/*******************************************************************************
 * This function retrieves SCR_EL3 member of 'cpu_context' pertaining to the
 * given security state.
 ******************************************************************************/
uint32_t cm_get_scr_el3(uint32_t security_state)
{
	cpu_context_t *ctx;
	el3_state_t *state;

	ctx = cm_get_context(security_state);
	assert(ctx);

	/* Populate EL3 state so that ERET jumps to the correct entry */
	state = get_el3state_ctx(ctx);
	return read_ctx_reg(state, CTX_SCR_EL3);
}

/*******************************************************************************
 * This function is used to program the context that's used for exception
 * return. This initializes the SP_EL3 to a pointer to a 'cpu_context' set for
 * the required security state
 ******************************************************************************/
void cm_set_next_eret_context(uint32_t security_state)
{
	cpu_context_t *ctx;

	ctx = cm_get_context(security_state);
	assert(ctx);

	cm_set_next_context(ctx);
}
