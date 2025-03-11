/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Dispatch synchronous system register traps from lower ELs.
 */

#include <arch_features.h>
#include <arch_helpers.h>
#include <bl31/sync_handle.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>

int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx)
{
	uint64_t __unused opcode = esr_el3 & ISS_SYSREG_OPCODE_MASK;

#if ENABLE_FEAT_RNG_TRAP
	if ((opcode == ISS_SYSREG_OPCODE_RNDR) || (opcode == ISS_SYSREG_OPCODE_RNDRRS)) {
		return plat_handle_rng_trap(esr_el3, ctx);
	}
#endif

#if IMPDEF_SYSREG_TRAP
	if ((opcode & ISS_SYSREG_OPCODE_IMPDEF) == ISS_SYSREG_OPCODE_IMPDEF) {
		return plat_handle_impdef_trap(esr_el3, ctx);
	}
#endif

	return TRAP_RET_UNHANDLED;
}

static bool is_tge_enabled(void)
{
	u_register_t hcr_el2 = read_hcr_el2();

	return ((is_feat_vhe_present()) && ((hcr_el2 & HCR_TGE_BIT) != 0U));
}

/*
 * This function is to ensure that undef injection does not happen into
 * non-existent S-EL2. This could happen when trap happens from S-EL{1,0}
 * and non-secure world is running with TGE bit set, considering EL3 does
 * not save/restore EL2 registers if only one world has EL2 enabled.
 * So reading hcr_el2.TGE would give NS world value.
 */
static bool is_secure_trap_without_sel2(u_register_t scr)
{
	return ((scr & (SCR_NS_BIT | SCR_EEL2_BIT)) == 0);
}

static unsigned int target_el(unsigned int from_el, u_register_t scr)
{
	if (from_el > MODE_EL1) {
		return from_el;
	} else if (is_tge_enabled() && !is_secure_trap_without_sel2(scr)) {
		return MODE_EL2;
	} else {
		return MODE_EL1;
	}
}

static u_register_t get_elr_el3(u_register_t spsr_el3, u_register_t vbar, unsigned int target_el)
{
	unsigned int outgoing_el = GET_EL(spsr_el3);
	u_register_t elr_el3 = 0;

	if (outgoing_el == target_el) {
		/*
		 * Target EL is either EL1 or EL2, lsb can tell us the SPsel
		 *  Thread mode  : 0
		 *  Handler mode : 1
		 */
		if ((spsr_el3 & (MODE_SP_MASK << MODE_SP_SHIFT)) == MODE_SP_ELX) {
			elr_el3 = vbar + CURRENT_EL_SPX;
		} else {
			elr_el3 = vbar + CURRENT_EL_SP0;
		}
	} else {
		/* Vector address for Lower EL using Aarch64 */
		elr_el3 = vbar + LOWER_EL_AARCH64;
	}

	return elr_el3;
}

/*
 * Explicitly create all bits of SPSR to get PSTATE at exception return.
 *
 * The code is based on "Aarch64.exceptions.takeexception" described in
 * DDI0602 revision 2025-03.
 * "https://developer.arm.com/documentation/ddi0597/2025-03/Shared-Pseudocode/
 * aarch64-exceptions-takeexception"
 *
 * NOTE: This piece of code must be reviewed every release against the latest
 * takeexception sequence to ensure that we keep up with new arch features that
 * affect the PSTATE.
 *
 * TF-A 2.13 release review
 *
 * Review of version 2025-03 indicates we are missing support for one feature.
 *  - FEAT_UINJ (2024 extension)
 */
u_register_t create_spsr(u_register_t old_spsr, unsigned int target_el)
{
	u_register_t new_spsr = 0;
	u_register_t sctlr;

	/* Set M bits for target EL in AArch64 mode, also get sctlr */
	if (target_el == MODE_EL2) {
		sctlr = read_sctlr_el2();
		new_spsr |= (SPSR_M_AARCH64 << SPSR_M_SHIFT) | SPSR_M_EL2H;
	} else {
		sctlr = read_sctlr_el1();
		new_spsr |= (SPSR_M_AARCH64 << SPSR_M_SHIFT) | SPSR_M_EL1H;
	}

	/* Mask all exceptions, update DAIF bits */
	new_spsr |= SPSR_DAIF_MASK << SPSR_DAIF_SHIFT;

	/* If FEAT_BTI is present, clear BTYPE bits */
	new_spsr |= old_spsr & (SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	if (is_feat_bti_present()) {
		new_spsr &= ~(SPSR_BTYPE_MASK_AARCH64 << SPSR_BTYPE_SHIFT_AARCH64);
	}

	/* If SSBS is implemented, take the value from SCTLR.DSSBS */
	new_spsr |= old_spsr & SPSR_SSBS_BIT_AARCH64;
	if (is_feat_ssbs_present()) {
		if ((sctlr & SCTLR_DSSBS_BIT) != 0U) {
			new_spsr |= SPSR_SSBS_BIT_AARCH64;
		} else {
			new_spsr &= ~SPSR_SSBS_BIT_AARCH64;
		}
	}

	/* If FEAT_NMI is implemented, ALLINT = !(SCTLR.SPINTMASK) */
	new_spsr |= old_spsr & SPSR_ALLINT_BIT_AARCH64;
	if (is_feat_nmi_present()) {
		if ((sctlr & SCTLR_SPINTMASK_BIT) != 0U) {
			new_spsr &= ~SPSR_ALLINT_BIT_AARCH64;
		} else {
			new_spsr |= SPSR_ALLINT_BIT_AARCH64;
		}
	}

	/* Clear PSTATE.IL bit explicitly */
	new_spsr &= ~SPSR_IL_BIT;

	/* Clear PSTATE.SS bit explicitly */
	new_spsr &= ~SPSR_SS_BIT;

	/* Update PSTATE.PAN bit */
	new_spsr |= old_spsr & SPSR_PAN_BIT;
	if (is_feat_pan_present() &&
	    ((target_el == MODE_EL1) || ((target_el == MODE_EL2) && is_tge_enabled())) &&
	    ((sctlr & SCTLR_SPAN_BIT) == 0U)) {
	    new_spsr |= SPSR_PAN_BIT;
	}

	/* Clear UAO bit if FEAT_UAO is present */
	new_spsr |= old_spsr & SPSR_UAO_BIT_AARCH64;
	if (is_feat_uao_present()) {
		new_spsr &= ~SPSR_UAO_BIT_AARCH64;
	}

	/* DIT bits are unchanged */
	new_spsr |= old_spsr & SPSR_DIT_BIT;

	/* If FEAT_MTE2 is implemented mask tag faults by setting TCO bit */
	new_spsr |= old_spsr & SPSR_TCO_BIT_AARCH64;
	if (is_feat_mte2_present()) {
		new_spsr |= SPSR_TCO_BIT_AARCH64;
	}

	/* NZCV bits are unchanged */
	new_spsr |= old_spsr & SPSR_NZCV;

	/* If FEAT_EBEP is present set PM bit */
	new_spsr |= old_spsr & SPSR_PM_BIT_AARCH64;
	if (is_feat_ebep_present()) {
		new_spsr |= SPSR_PM_BIT_AARCH64;
	}

	/* If FEAT_SEBEP is present clear PPEND bit */
	new_spsr |= old_spsr & SPSR_PPEND_BIT;
	if (is_feat_sebep_present()) {
		new_spsr &= ~SPSR_PPEND_BIT;
	}

	/* If FEAT_GCS is present, update EXLOCK bit */
	new_spsr |= old_spsr & SPSR_EXLOCK_BIT_AARCH64;
	if (is_feat_gcs_present()) {
		u_register_t gcscr;
		if (target_el == MODE_EL2) {
			gcscr = read_gcscr_el2();
		} else {
			gcscr = read_gcscr_el1();
		}
		new_spsr |= (gcscr & GCSCR_EXLOCK_EN_BIT) ? SPSR_EXLOCK_BIT_AARCH64 : 0;
	}

	/* If FEAT_PAUTH_LR present then zero the PACM bit. */
	new_spsr |= old_spsr & SPSR_PACM_BIT_AARCH64;
	if (is_feat_pauth_lr_present()) {
		new_spsr &= ~SPSR_PACM_BIT_AARCH64;
	}

	return new_spsr;
}

/*
 * Handler for injecting Undefined exception to lower EL which is caused by
 * lower EL accessing system registers of which (old)EL3 firmware is unaware.
 *
 * This is a safety net to avoid EL3 panics caused by system register access
 * that triggers an exception syndrome EC=0x18.
 */
void inject_undef64(cpu_context_t *ctx)
{
	u_register_t esr = (EC_UNKNOWN << ESR_EC_SHIFT) | ESR_IL_BIT;
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t elr_el3 = read_ctx_reg(state, CTX_ELR_EL3);
	u_register_t old_spsr = read_ctx_reg(state, CTX_SPSR_EL3);
	u_register_t scr_el3 = read_ctx_reg(state, CTX_SCR_EL3);
	u_register_t new_spsr = 0;
	unsigned int to_el = target_el(GET_EL(old_spsr), scr_el3);

	if (to_el == MODE_EL2) {
		write_elr_el2(elr_el3);
		elr_el3 = get_elr_el3(old_spsr, read_vbar_el2(), to_el);
		write_esr_el2(esr);
		write_spsr_el2(old_spsr);
	} else {
		write_elr_el1(elr_el3);
		elr_el3 = get_elr_el3(old_spsr, read_vbar_el1(), to_el);
		write_esr_el1(esr);
		write_spsr_el1(old_spsr);
	}

	new_spsr = create_spsr(old_spsr, to_el);

	write_ctx_reg(state, CTX_SPSR_EL3, new_spsr);
	write_ctx_reg(state, CTX_ELR_EL3, elr_el3);
}
