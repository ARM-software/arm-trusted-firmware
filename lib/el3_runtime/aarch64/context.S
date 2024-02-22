/*
 * Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <context.h>
#include <el3_common_macros.S>

#if CTX_INCLUDE_FPREGS
	.global	fpregs_context_save
	.global	fpregs_context_restore
#endif /* CTX_INCLUDE_FPREGS */
	.global	prepare_el3_entry
	.global	restore_gp_pmcr_pauth_regs
	.global save_and_update_ptw_el1_sys_regs
	.global	el3_exit

/* ------------------------------------------------------------------
 * The following function follows the aapcs_64 strictly to use
 * x9-x17 (temporary caller-saved registers according to AArch64 PCS)
 * to save floating point register context. It assumes that 'x0' is
 * pointing to a 'fp_regs' structure where the register context will
 * be saved.
 *
 * Access to VFP registers will trap if CPTR_EL3.TFP is set.
 * However currently we don't use VFP registers nor set traps in
 * Trusted Firmware, and assume it's cleared.
 *
 * TODO: Revisit when VFP is used in secure world
 * ------------------------------------------------------------------
 */
#if CTX_INCLUDE_FPREGS
func fpregs_context_save
	stp	q0, q1, [x0, #CTX_FP_Q0]
	stp	q2, q3, [x0, #CTX_FP_Q2]
	stp	q4, q5, [x0, #CTX_FP_Q4]
	stp	q6, q7, [x0, #CTX_FP_Q6]
	stp	q8, q9, [x0, #CTX_FP_Q8]
	stp	q10, q11, [x0, #CTX_FP_Q10]
	stp	q12, q13, [x0, #CTX_FP_Q12]
	stp	q14, q15, [x0, #CTX_FP_Q14]
	stp	q16, q17, [x0, #CTX_FP_Q16]
	stp	q18, q19, [x0, #CTX_FP_Q18]
	stp	q20, q21, [x0, #CTX_FP_Q20]
	stp	q22, q23, [x0, #CTX_FP_Q22]
	stp	q24, q25, [x0, #CTX_FP_Q24]
	stp	q26, q27, [x0, #CTX_FP_Q26]
	stp	q28, q29, [x0, #CTX_FP_Q28]
	stp	q30, q31, [x0, #CTX_FP_Q30]

	mrs	x9, fpsr
	str	x9, [x0, #CTX_FP_FPSR]

	mrs	x10, fpcr
	str	x10, [x0, #CTX_FP_FPCR]

#if CTX_INCLUDE_AARCH32_REGS
	mrs	x11, fpexc32_el2
	str	x11, [x0, #CTX_FP_FPEXC32_EL2]
#endif /* CTX_INCLUDE_AARCH32_REGS */
	ret
endfunc fpregs_context_save

/* ------------------------------------------------------------------
 * The following function follows the aapcs_64 strictly to use x9-x17
 * (temporary caller-saved registers according to AArch64 PCS) to
 * restore floating point register context. It assumes that 'x0' is
 * pointing to a 'fp_regs' structure from where the register context
 * will be restored.
 *
 * Access to VFP registers will trap if CPTR_EL3.TFP is set.
 * However currently we don't use VFP registers nor set traps in
 * Trusted Firmware, and assume it's cleared.
 *
 * TODO: Revisit when VFP is used in secure world
 * ------------------------------------------------------------------
 */
func fpregs_context_restore
	ldp	q0, q1, [x0, #CTX_FP_Q0]
	ldp	q2, q3, [x0, #CTX_FP_Q2]
	ldp	q4, q5, [x0, #CTX_FP_Q4]
	ldp	q6, q7, [x0, #CTX_FP_Q6]
	ldp	q8, q9, [x0, #CTX_FP_Q8]
	ldp	q10, q11, [x0, #CTX_FP_Q10]
	ldp	q12, q13, [x0, #CTX_FP_Q12]
	ldp	q14, q15, [x0, #CTX_FP_Q14]
	ldp	q16, q17, [x0, #CTX_FP_Q16]
	ldp	q18, q19, [x0, #CTX_FP_Q18]
	ldp	q20, q21, [x0, #CTX_FP_Q20]
	ldp	q22, q23, [x0, #CTX_FP_Q22]
	ldp	q24, q25, [x0, #CTX_FP_Q24]
	ldp	q26, q27, [x0, #CTX_FP_Q26]
	ldp	q28, q29, [x0, #CTX_FP_Q28]
	ldp	q30, q31, [x0, #CTX_FP_Q30]

	ldr	x9, [x0, #CTX_FP_FPSR]
	msr	fpsr, x9

	ldr	x10, [x0, #CTX_FP_FPCR]
	msr	fpcr, x10

#if CTX_INCLUDE_AARCH32_REGS
	ldr	x11, [x0, #CTX_FP_FPEXC32_EL2]
	msr	fpexc32_el2, x11
#endif /* CTX_INCLUDE_AARCH32_REGS */

	/*
	 * No explict ISB required here as ERET to
	 * switch to secure EL1 or non-secure world
	 * covers it
	 */

	ret
endfunc fpregs_context_restore
#endif /* CTX_INCLUDE_FPREGS */

	/*
	 * Set SCR_EL3.EA bit to enable SErrors at EL3
	 */
	.macro enable_serror_at_el3
	mrs     x8, scr_el3
	orr     x8, x8, #SCR_EA_BIT
	msr     scr_el3, x8
	.endm

	/*
	 * Set the PSTATE bits not set when the exception was taken as
	 * described in the AArch64.TakeException() pseudocode function
	 * in ARM DDI 0487F.c page J1-7635 to a default value.
	 */
	.macro set_unset_pstate_bits
	/*
	 * If Data Independent Timing (DIT) functionality is implemented,
	 * always enable DIT in EL3
	 */
#if ENABLE_FEAT_DIT
#if ENABLE_FEAT_DIT == 2
	mrs	x8, id_aa64pfr0_el1
	and	x8, x8, #(ID_AA64PFR0_DIT_MASK << ID_AA64PFR0_DIT_SHIFT)
	cbz	x8, 1f
#endif
	mov     x8, #DIT_BIT
	msr     DIT, x8
1:
#endif /* ENABLE_FEAT_DIT */
	.endm /* set_unset_pstate_bits */

/*-------------------------------------------------------------------------
 * This macro checks the ENABLE_FEAT_MPAM state, performs ID register
 * check to see if the platform supports MPAM extension and restores MPAM3
 * register value if it is FEAT_STATE_ENABLED/FEAT_STATE_CHECKED.
 *
 * This is particularly more complicated because we can't check
 * if the platform supports MPAM  by looking for status of a particular bit
 * in the MDCR_EL3 or CPTR_EL3 register like other extensions.
 * ------------------------------------------------------------------------
 */

	.macro	restore_mpam3_el3
#if ENABLE_FEAT_MPAM
#if ENABLE_FEAT_MPAM == 2

	mrs x8, id_aa64pfr0_el1
	lsr x8, x8, #(ID_AA64PFR0_MPAM_SHIFT)
	and x8, x8, #(ID_AA64PFR0_MPAM_MASK)
	mrs x7, id_aa64pfr1_el1
	lsr x7, x7, #(ID_AA64PFR1_MPAM_FRAC_SHIFT)
	and x7, x7, #(ID_AA64PFR1_MPAM_FRAC_MASK)
	orr x7, x7, x8
	cbz x7, no_mpam
#endif
	/* -----------------------------------------------------------
	 * Restore MPAM3_EL3 register as per context state
	 * Currently we only enable MPAM for NS world and trap to EL3
	 * for MPAM access in lower ELs of Secure and Realm world
	 * x9 holds address of the per_world context
	 * -----------------------------------------------------------
	 */

	ldr	x17, [x9, #CTX_MPAM3_EL3]
	msr	S3_6_C10_C5_0, x17 /* mpam3_el3 */

no_mpam:
#endif
	.endm /* restore_mpam3_el3 */

/* ------------------------------------------------------------------
 * The following macro is used to save and restore all the general
 * purpose and ARMv8.3-PAuth (if enabled) registers.
 * It also checks if the Secure Cycle Counter (PMCCNTR_EL0)
 * is disabled in EL3/Secure (ARMv8.5-PMU), wherein PMCCNTR_EL0
 * needs not to be saved/restored during world switch.
 *
 * Ideally we would only save and restore the callee saved registers
 * when a world switch occurs but that type of implementation is more
 * complex. So currently we will always save and restore these
 * registers on entry and exit of EL3.
 * clobbers: x18
 * ------------------------------------------------------------------
 */
	.macro save_gp_pmcr_pauth_regs
	stp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	stp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	stp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]
	stp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	stp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	stp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	stp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	stp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]
	stp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	stp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	stp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	stp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	stp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	stp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]
	mrs	x18, sp_el0
	str	x18, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_SP_EL0]

	/* PMUv3 is presumed to be always present */
	mrs	x9, pmcr_el0
	str	x9, [sp, #CTX_EL3STATE_OFFSET + CTX_PMCR_EL0]
	/* Disable cycle counter when event counting is prohibited */
	orr	x9, x9, #PMCR_EL0_DP_BIT
	msr	pmcr_el0, x9
	isb
#if CTX_INCLUDE_PAUTH_REGS
	/* ----------------------------------------------------------
 	 * Save the ARMv8.3-PAuth keys as they are not banked
 	 * by exception level
	 * ----------------------------------------------------------
	 */
	add	x19, sp, #CTX_PAUTH_REGS_OFFSET

	mrs	x20, APIAKeyLo_EL1	/* x21:x20 = APIAKey */
	mrs	x21, APIAKeyHi_EL1
	mrs	x22, APIBKeyLo_EL1	/* x23:x22 = APIBKey */
	mrs	x23, APIBKeyHi_EL1
	mrs	x24, APDAKeyLo_EL1	/* x25:x24 = APDAKey */
	mrs	x25, APDAKeyHi_EL1
	mrs	x26, APDBKeyLo_EL1	/* x27:x26 = APDBKey */
	mrs	x27, APDBKeyHi_EL1
	mrs	x28, APGAKeyLo_EL1	/* x29:x28 = APGAKey */
	mrs	x29, APGAKeyHi_EL1

	stp	x20, x21, [x19, #CTX_PACIAKEY_LO]
	stp	x22, x23, [x19, #CTX_PACIBKEY_LO]
	stp	x24, x25, [x19, #CTX_PACDAKEY_LO]
	stp	x26, x27, [x19, #CTX_PACDBKEY_LO]
	stp	x28, x29, [x19, #CTX_PACGAKEY_LO]
#endif /* CTX_INCLUDE_PAUTH_REGS */
	.endm /* save_gp_pmcr_pauth_regs */

/* -----------------------------------------------------------------
 * This function saves the context and sets the PSTATE to a known
 * state, preparing entry to el3.
 * Save all the general purpose and ARMv8.3-PAuth (if enabled)
 * registers.
 * Then set any of the PSTATE bits that are not set by hardware
 * according to the Aarch64.TakeException pseudocode in the Arm
 * Architecture Reference Manual to a default value for EL3.
 * clobbers: x17
 * -----------------------------------------------------------------
 */
func prepare_el3_entry
	save_gp_pmcr_pauth_regs
	enable_serror_at_el3
	/*
	 * Set the PSTATE bits not described in the Aarch64.TakeException
	 * pseudocode to their default values.
	 */
	set_unset_pstate_bits
	ret
endfunc prepare_el3_entry

/* ------------------------------------------------------------------
 * This function restores ARMv8.3-PAuth (if enabled) and all general
 * purpose registers except x30 from the CPU context.
 * x30 register must be explicitly restored by the caller.
 * ------------------------------------------------------------------
 */
func restore_gp_pmcr_pauth_regs
#if CTX_INCLUDE_PAUTH_REGS
 	/* Restore the ARMv8.3 PAuth keys */
	add	x10, sp, #CTX_PAUTH_REGS_OFFSET

	ldp	x0, x1, [x10, #CTX_PACIAKEY_LO]	/* x1:x0 = APIAKey */
	ldp	x2, x3, [x10, #CTX_PACIBKEY_LO]	/* x3:x2 = APIBKey */
	ldp	x4, x5, [x10, #CTX_PACDAKEY_LO]	/* x5:x4 = APDAKey */
	ldp	x6, x7, [x10, #CTX_PACDBKEY_LO]	/* x7:x6 = APDBKey */
	ldp	x8, x9, [x10, #CTX_PACGAKEY_LO]	/* x9:x8 = APGAKey */

	msr	APIAKeyLo_EL1, x0
	msr	APIAKeyHi_EL1, x1
	msr	APIBKeyLo_EL1, x2
	msr	APIBKeyHi_EL1, x3
	msr	APDAKeyLo_EL1, x4
	msr	APDAKeyHi_EL1, x5
	msr	APDBKeyLo_EL1, x6
	msr	APDBKeyHi_EL1, x7
	msr	APGAKeyLo_EL1, x8
	msr	APGAKeyHi_EL1, x9
#endif /* CTX_INCLUDE_PAUTH_REGS */

	/* PMUv3 is presumed to be always present */
	ldr	x0, [sp, #CTX_EL3STATE_OFFSET + CTX_PMCR_EL0]
	msr	pmcr_el0, x0
	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	ldp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	ldp	x6, x7, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X6]
	ldp	x8, x9, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X8]
	ldp	x10, x11, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X10]
	ldp	x12, x13, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X12]
	ldp	x14, x15, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X14]
	ldp	x16, x17, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X16]
	ldp	x18, x19, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X18]
	ldp	x20, x21, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X20]
	ldp	x22, x23, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X22]
	ldp	x24, x25, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X24]
	ldp	x26, x27, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X26]
	ldr	x28, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_SP_EL0]
	msr	sp_el0, x28
	ldp	x28, x29, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X28]
	ret
endfunc restore_gp_pmcr_pauth_regs

/*
 * In case of ERRATA_SPECULATIVE_AT, save SCTLR_EL1 and TCR_EL1
 * registers and update EL1 registers to disable stage1 and stage2
 * page table walk
 */
func save_and_update_ptw_el1_sys_regs
	/* ----------------------------------------------------------
	 * Save only sctlr_el1 and tcr_el1 registers
	 * ----------------------------------------------------------
	 */
	mrs	x29, sctlr_el1
	str	x29, [sp, #(CTX_EL1_SYSREGS_OFFSET + CTX_SCTLR_EL1)]
	mrs	x29, tcr_el1
	str	x29, [sp, #(CTX_EL1_SYSREGS_OFFSET + CTX_TCR_EL1)]

	/* ------------------------------------------------------------
	 * Must follow below order in order to disable page table
	 * walk for lower ELs (EL1 and EL0). First step ensures that
	 * page table walk is disabled for stage1 and second step
	 * ensures that page table walker should use TCR_EL1.EPDx
	 * bits to perform address translation. ISB ensures that CPU
	 * does these 2 steps in order.
	 *
	 * 1. Update TCR_EL1.EPDx bits to disable page table walk by
	 *    stage1.
	 * 2. Enable MMU bit to avoid identity mapping via stage2
	 *    and force TCR_EL1.EPDx to be used by the page table
	 *    walker.
	 * ------------------------------------------------------------
	 */
	orr	x29, x29, #(TCR_EPD0_BIT)
	orr	x29, x29, #(TCR_EPD1_BIT)
	msr	tcr_el1, x29
	isb
	mrs	x29, sctlr_el1
	orr	x29, x29, #SCTLR_M_BIT
	msr	sctlr_el1, x29
	isb

	ret
endfunc save_and_update_ptw_el1_sys_regs

/* -----------------------------------------------------------------
* The below macro returns the address of the per_world context for
* the security state, retrieved through "get_security_state" macro.
* The per_world context address is returned in the register argument.
* Clobbers: x9, x10
* ------------------------------------------------------------------
*/

.macro get_per_world_context _reg:req
	ldr 	x10, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]
	get_security_state x9, x10
	mov_imm	x10, (CTX_PERWORLD_EL3STATE_END - CTX_CPTR_EL3)
	mul	x9, x9, x10
	adrp	x10, per_world_context
	add	x10, x10, :lo12:per_world_context
	add	x9, x9, x10
	mov 	\_reg, x9
.endm

/* ------------------------------------------------------------------
 * This routine assumes that the SP_EL3 is pointing to a valid
 * context structure from where the gp regs and other special
 * registers can be retrieved.
 * ------------------------------------------------------------------
 */
func el3_exit
#if ENABLE_ASSERTIONS
	/* el3_exit assumes SP_EL0 on entry */
	mrs	x17, spsel
	cmp	x17, #MODE_SP_EL0
	ASM_ASSERT(eq)
#endif /* ENABLE_ASSERTIONS */

	/* ----------------------------------------------------------
	 * Save the current SP_EL0 i.e. the EL3 runtime stack which
	 * will be used for handling the next SMC.
	 * Then switch to SP_EL3.
	 * ----------------------------------------------------------
	 */
	mov	x17, sp
	msr	spsel, #MODE_SP_ELX
	str	x17, [sp, #CTX_EL3STATE_OFFSET + CTX_RUNTIME_SP]

	/* ----------------------------------------------------------
	 * Restore CPTR_EL3.
	 * ZCR is only restored if SVE is supported and enabled.
	 * Synchronization is required before zcr_el3 is addressed.
	 * ----------------------------------------------------------
	 */

	/* The address of the per_world context is stored in x9 */
	get_per_world_context x9

	ldp	x19, x20, [x9, #CTX_CPTR_EL3]
	msr	cptr_el3, x19

#if IMAGE_BL31
	ands	x19, x19, #CPTR_EZ_BIT
	beq	sve_not_enabled

	isb
	msr	S3_6_C1_C2_0, x20 /* zcr_el3 */
sve_not_enabled:

	restore_mpam3_el3

#endif /* IMAGE_BL31 */

#if IMAGE_BL31 && DYNAMIC_WORKAROUND_CVE_2018_3639
	/* ----------------------------------------------------------
	 * Restore mitigation state as it was on entry to EL3
	 * ----------------------------------------------------------
	 */
	ldr	x17, [sp, #CTX_CVE_2018_3639_OFFSET + CTX_CVE_2018_3639_DISABLE]
	cbz	x17, 1f
	blr	x17
1:
#endif /* IMAGE_BL31 && DYNAMIC_WORKAROUND_CVE_2018_3639 */

#if IMAGE_BL31
	synchronize_errors
#endif /* IMAGE_BL31 */

	/* ----------------------------------------------------------
	 * Restore SPSR_EL3, ELR_EL3 and SCR_EL3 prior to ERET
	 * ----------------------------------------------------------
	 */
	ldr	x18, [sp, #CTX_EL3STATE_OFFSET + CTX_SCR_EL3]
	ldp	x16, x17, [sp, #CTX_EL3STATE_OFFSET + CTX_SPSR_EL3]
	msr	scr_el3, x18
	msr	spsr_el3, x16
	msr	elr_el3, x17

	restore_ptw_el1_sys_regs

	/* ----------------------------------------------------------
	 * Restore general purpose (including x30), PMCR_EL0 and
	 * ARMv8.3-PAuth registers.
	 * Exit EL3 via ERET to a lower exception level.
 	 * ----------------------------------------------------------
 	 */
	bl	restore_gp_pmcr_pauth_regs
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

#ifdef IMAGE_BL31
	/* Clear the EL3 flag as we are exiting el3 */
	str	xzr, [sp, #CTX_EL3STATE_OFFSET + CTX_NESTED_EA_FLAG]
#endif /* IMAGE_BL31 */

	exception_return

endfunc el3_exit
