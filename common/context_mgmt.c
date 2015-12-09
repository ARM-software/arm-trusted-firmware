/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
#include <interrupt_mgmt.h>
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
	uint32_t scr_el3;
	el3_state_t *state;
	gp_regs_t *gp_regs;
	unsigned long sctlr_elx;

	assert(ctx);

	security_state = GET_SECURITY_STATE(ep->h.attr);

	/* Clear any residual register values from the context */
	memset(ctx, 0, sizeof(*ctx));

	/*
	 * Base the context SCR on the current value, adjust for entry point
	 * specific requirements and set trap bits from the IMF
	 * TODO: provide the base/global SCR bits using another mechanism?
	 */
	scr_el3 = read_scr();
	scr_el3 &= ~(SCR_NS_BIT | SCR_RW_BIT | SCR_FIQ_BIT | SCR_IRQ_BIT |
			SCR_ST_BIT | SCR_HCE_BIT);

	if (security_state != SECURE)
		scr_el3 |= SCR_NS_BIT;

	if (GET_RW(ep->spsr) == MODE_RW_64)
		scr_el3 |= SCR_RW_BIT;

	if (EP_GET_ST(ep->h.attr))
		scr_el3 |= SCR_ST_BIT;

#if IMAGE_BL31
	/*
	 * IRQ/FIQ bits only need setting if interrupt routing
	 * model has been set up for BL31.
	 */
	scr_el3 |= get_scr_el3_from_routing_model(security_state);
#endif

	/*
	 * Set up SCTLR_ELx for the target exception level:
	 * EE bit is taken from the entrpoint attributes
	 * M, C and I bits must be zero (as required by PSCI specification)
	 *
	 * The target exception level is based on the spsr mode requested.
	 * If execution is requested to EL2 or hyp mode, HVC is enabled
	 * via SCR_EL3.HCE.
	 *
	 * Always compute the SCTLR_EL1 value and save in the cpu_context
	 * - the EL2 registers are set up by cm_preapre_ns_entry() as they
	 * are not part of the stored cpu_context
	 *
	 * TODO: In debug builds the spsr should be validated and checked
	 * against the CPU support, security state, endianess and pc
	 */
	sctlr_elx = EP_GET_EE(ep->h.attr) ? SCTLR_EE_BIT : 0;
	if (GET_RW(ep->spsr) == MODE_RW_64)
		sctlr_elx |= SCTLR_EL1_RES1;
	else
		sctlr_elx |= SCTLR_AARCH32_EL1_RES1;
	write_ctx_reg(get_sysregs_ctx(ctx), CTX_SCTLR_EL1, sctlr_elx);

	if ((GET_RW(ep->spsr) == MODE_RW_64
	     && GET_EL(ep->spsr) == MODE_EL2)
	    || (GET_RW(ep->spsr) != MODE_RW_64
		&& GET_M32(ep->spsr) == MODE32_hyp)) {
		scr_el3 |= SCR_HCE_BIT;
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
	uint32_t sctlr_elx, scr_el3, cptr_el2;
	cpu_context_t *ctx = cm_get_context(security_state);

	assert(ctx);

	if (security_state == NON_SECURE) {
		scr_el3 = read_ctx_reg(get_el3state_ctx(ctx), CTX_SCR_EL3);
		if (scr_el3 & SCR_HCE_BIT) {
			/* Use SCTLR_EL1.EE value to initialise sctlr_el2 */
			sctlr_elx = read_ctx_reg(get_sysregs_ctx(ctx),
						 CTX_SCTLR_EL1);
			sctlr_elx &= ~SCTLR_EE_BIT;
			sctlr_elx |= SCTLR_EL2_RES1;
			write_sctlr_el2(sctlr_elx);
		} else if (read_id_aa64pfr0_el1() &
			   (ID_AA64PFR0_ELX_MASK << ID_AA64PFR0_EL2_SHIFT)) {
			/* EL2 present but unused, need to disable safely */

			/* HCR_EL2 = 0, except RW bit set to match SCR_EL3 */
			write_hcr_el2((scr_el3 & SCR_RW_BIT) ? HCR_RW_BIT : 0);

			/* SCTLR_EL2 : can be ignored when bypassing */

			/* CPTR_EL2 : disable all traps TCPAC, TTA, TFP */
			cptr_el2 = read_cptr_el2();
			cptr_el2 &= ~(TCPAC_BIT | TTA_BIT | TFP_BIT);
			write_cptr_el2(cptr_el2);

			/* Enable EL1 access to timer */
			write_cnthctl_el2(EL1PCEN_BIT | EL1PCTEN_BIT);

			/* Reset CNTVOFF_EL2 */
			write_cntvoff_el2(0);

			/* Set VPIDR, VMPIDR to match MIDR, MPIDR */
			write_vpidr_el2(read_midr_el1());
			write_vmpidr_el2(read_mpidr_el1());

			/*
			 * Reset VTTBR_EL2.
			 * Needed because cache maintenance operations depend on
			 * the VMID even when non-secure EL1&0 stage 2 address
			 * translation are disabled.
			 */
			write_vttbr_el2(0);
		}
	}

	el1_sysregs_context_restore(get_sysregs_ctx(ctx));

	cm_set_next_context(ctx);
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
}

void cm_el1_sysregs_context_restore(uint32_t security_state)
{
	cpu_context_t *ctx;

	ctx = cm_get_context(security_state);
	assert(ctx);

	el1_sysregs_context_restore(get_sysregs_ctx(ctx));
}

/*******************************************************************************
 * This function populates ELR_EL3 member of 'cpu_context' pertaining to the
 * given security state with the given entrypoint
 ******************************************************************************/
void cm_set_elr_el3(uint32_t security_state, uint64_t entrypoint)
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
			 uint64_t entrypoint, uint32_t spsr)
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
