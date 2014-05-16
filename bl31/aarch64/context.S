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

#include <arch.h>
#include <asm_macros.S>
#include <context.h>

/* -----------------------------------------------------
 * The following function strictly follows the AArch64
 * PCS to use x9-x17 (temporary caller-saved registers)
 * to save essential EL3 system register context. It
 * assumes that 'x0' is pointing to a 'el1_sys_regs'
 * structure where the register context will be saved.
 * -----------------------------------------------------
 */
	.global el3_sysregs_context_save
func el3_sysregs_context_save

	mrs	x9, scr_el3
	mrs	x10, sctlr_el3
	stp	x9, x10, [x0, #CTX_SCR_EL3]

	mrs	x11, cptr_el3
	stp	x11, xzr, [x0, #CTX_CPTR_EL3]

	mrs	x13, cntfrq_el0
	mrs	x14, mair_el3
	stp	x13, x14, [x0, #CTX_CNTFRQ_EL0]

	mrs	x15, tcr_el3
	mrs	x16, ttbr0_el3
	stp	x15, x16, [x0, #CTX_TCR_EL3]

	mrs	x17, daif
	and	x17, x17, #(DAIF_ABT_BIT | DAIF_DBG_BIT)
	stp	x17, xzr, [x0, #CTX_DAIF_EL3]

	ret

/* -----------------------------------------------------
 * The following function strictly follows the AArch64
 * PCS to use x9-x17 (temporary caller-saved registers)
 * to restore essential EL3 system register context. It
 * assumes that 'x0' is pointing to a 'el1_sys_regs'
 * structure from where the register context will be
 * restored.
 *
 * Note that the sequence differs from that of the save
 * function as we want the MMU to be enabled last
 * -----------------------------------------------------
 */
	.global el3_sysregs_context_restore
func el3_sysregs_context_restore

	ldp	x11, xzr, [x0, #CTX_CPTR_EL3]
	msr	cptr_el3, x11

	ldp	x13, x14, [x0, #CTX_CNTFRQ_EL0]
	msr	cntfrq_el0, x13
	msr	mair_el3, x14

	ldp	x15, x16, [x0, #CTX_TCR_EL3]
	msr	tcr_el3, x15
	msr	ttbr0_el3, x16

	ldp	x17, xzr, [x0, #CTX_DAIF_EL3]
	mrs	x11, daif
	orr	x17, x17, x11
	msr	daif, x17

	/* Make sure all the above changes are observed */
	isb

	ldp	x9, x10, [x0, #CTX_SCR_EL3]
	msr	scr_el3, x9
	msr	sctlr_el3, x10
	isb

	ret

/* -----------------------------------------------------
 * The following function strictly follows the AArch64
 * PCS to use x9-x17 (temporary caller-saved registers)
 * to save EL1 system register context. It assumes that
 * 'x0' is pointing to a 'el1_sys_regs' structure where
 * the register context will be saved.
 * -----------------------------------------------------
 */
	.global el1_sysregs_context_save
func el1_sysregs_context_save

	mrs	x9, spsr_el1
	mrs	x10, elr_el1
	stp	x9, x10, [x0, #CTX_SPSR_EL1]

	mrs	x11, spsr_abt
	mrs	x12, spsr_und
	stp	x11, x12, [x0, #CTX_SPSR_ABT]

	mrs	x13, spsr_irq
	mrs	x14, spsr_fiq
	stp	x13, x14, [x0, #CTX_SPSR_IRQ]

	mrs	x15, sctlr_el1
	mrs	x16, actlr_el1
	stp	x15, x16, [x0, #CTX_SCTLR_EL1]

	mrs	x17, cpacr_el1
	mrs	x9, csselr_el1
	stp	x17, x9, [x0, #CTX_CPACR_EL1]

	mrs	x10, sp_el1
	mrs	x11, esr_el1
	stp	x10, x11, [x0, #CTX_SP_EL1]

	mrs	x12, ttbr0_el1
	mrs	x13, ttbr1_el1
	stp	x12, x13, [x0, #CTX_TTBR0_EL1]

	mrs	x14, mair_el1
	mrs	x15, amair_el1
	stp	x14, x15, [x0, #CTX_MAIR_EL1]

	mrs	x16, tcr_el1
	mrs	x17, tpidr_el1
	stp	x16, x17, [x0, #CTX_TCR_EL1]

	mrs	x9, tpidr_el0
	mrs	x10, tpidrro_el0
	stp	x9, x10, [x0, #CTX_TPIDR_EL0]

	mrs	x11, dacr32_el2
	mrs	x12, ifsr32_el2
	stp	x11, x12, [x0, #CTX_DACR32_EL2]

	mrs	x13, par_el1
	mrs	x14, far_el1
	stp	x13, x14, [x0, #CTX_PAR_EL1]

	mrs	x15, afsr0_el1
	mrs	x16, afsr1_el1
	stp	x15, x16, [x0, #CTX_AFSR0_EL1]

	mrs	x17, contextidr_el1
	mrs	x9, vbar_el1
	stp	x17, x9, [x0, #CTX_CONTEXTIDR_EL1]

	/* Save NS timer registers if the build has instructed so */
#if NS_TIMER_SWITCH
	mrs	x10, cntp_ctl_el0
	mrs	x11, cntp_cval_el0
	stp	x10, x11, [x0, #CTX_CNTP_CTL_EL0]

	mrs	x12, cntv_ctl_el0
	mrs	x13, cntv_cval_el0
	stp	x12, x13, [x0, #CTX_CNTV_CTL_EL0]

	mrs	x14, cntkctl_el1
	str	x14, [x0, #CTX_CNTKCTL_EL1]
#endif

	mrs	x15, fpexc32_el2
	str	x15, [x0, #CTX_FP_FPEXC32_EL2]

	ret

/* -----------------------------------------------------
 * The following function strictly follows the AArch64
 * PCS to use x9-x17 (temporary caller-saved registers)
 * to restore EL1 system register context.  It assumes
 * that 'x0' is pointing to a 'el1_sys_regs' structure
 * from where the register context will be restored
 * -----------------------------------------------------
 */
	.global el1_sysregs_context_restore
func el1_sysregs_context_restore

	ldp	x9, x10, [x0, #CTX_SPSR_EL1]
	msr	spsr_el1, x9
	msr	elr_el1, x10

	ldp	x11, x12, [x0, #CTX_SPSR_ABT]
	msr	spsr_abt, x11
	msr	spsr_und, x12

	ldp	x13, x14, [x0, #CTX_SPSR_IRQ]
	msr	spsr_irq, x13
	msr	spsr_fiq, x14

	ldp	x15, x16, [x0, #CTX_SCTLR_EL1]
	msr	sctlr_el1, x15
	msr	actlr_el1, x16

	ldp	x17, x9, [x0, #CTX_CPACR_EL1]
	msr	cpacr_el1, x17
	msr	csselr_el1, x9

	ldp	x10, x11, [x0, #CTX_SP_EL1]
	msr	sp_el1, x10
	msr	esr_el1, x11

	ldp	x12, x13, [x0, #CTX_TTBR0_EL1]
	msr	ttbr0_el1, x12
	msr	ttbr1_el1, x13

	ldp	x14, x15, [x0, #CTX_MAIR_EL1]
	msr	mair_el1, x14
	msr	amair_el1, x15

	ldp	x16, x17, [x0, #CTX_TCR_EL1]
	msr	tcr_el1, x16
	msr	tpidr_el1, x17

	ldp	x9, x10, [x0, #CTX_TPIDR_EL0]
	msr	tpidr_el0, x9
	msr	tpidrro_el0, x10

	ldp	x11, x12, [x0, #CTX_DACR32_EL2]
	msr	dacr32_el2, x11
	msr	ifsr32_el2, x12

	ldp	x13, x14, [x0, #CTX_PAR_EL1]
	msr	par_el1, x13
	msr	far_el1, x14

	ldp	x15, x16, [x0, #CTX_AFSR0_EL1]
	msr	afsr0_el1, x15
	msr	afsr1_el1, x16

	ldp	x17, x9, [x0, #CTX_CONTEXTIDR_EL1]
	msr	contextidr_el1, x17
	msr	vbar_el1, x9

	/* Restore NS timer registers if the build has instructed so */
#if NS_TIMER_SWITCH
	ldp	x10, x11, [x0, #CTX_CNTP_CTL_EL0]
	msr	cntp_ctl_el0, x10
	msr	cntp_cval_el0, x11

	ldp	x12, x13, [x0, #CTX_CNTV_CTL_EL0]
	msr	cntv_ctl_el0, x12
	msr	cntv_cval_el0, x13

	ldr	x14, [x0, #CTX_CNTKCTL_EL1]
	msr	cntkctl_el1, x14
#endif

	ldr	x15, [x0, #CTX_FP_FPEXC32_EL2]
	msr	fpexc32_el2, x15

	/* No explict ISB required here as ERET covers it */

	ret

/* -----------------------------------------------------
 * The followsing function follows the aapcs_64 strictly
 * to use x9-x17 (temporary caller-saved registers
 * according to AArch64 PCS) to save floating point
 * register context. It assumes that 'x0' is pointing to
 * a 'fp_regs' structure where the register context will
 * be saved.
 *
 * Access to VFP registers will trap if CPTR_EL3.TFP is
 * set.  However currently we don't use VFP registers
 * nor set traps in Trusted Firmware, and assume it's
 * cleared
 *
 * TODO: Revisit when VFP is used in secure world
 * -----------------------------------------------------
 */
	.global fpregs_context_save
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

	ret

/* -----------------------------------------------------
 * The following function follows the aapcs_64 strictly
 * to use x9-x17 (temporary caller-saved registers
 * according to AArch64 PCS) to restore floating point
 * register context. It assumes that 'x0' is pointing to
 * a 'fp_regs' structure from where the register context
 * will be restored.
 *
 * Access to VFP registers will trap if CPTR_EL3.TFP is
 * set.  However currently we don't use VFP registers
 * nor set traps in Trusted Firmware, and assume it's
 * cleared
 *
 * TODO: Revisit when VFP is used in secure world
 * -----------------------------------------------------
 */
	.global fpregs_context_restore
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

	str	x10, [x0, #CTX_FP_FPCR]
	msr	fpcr, x10

	/*
	 * No explict ISB required here as ERET to
	 * swtich to secure EL1 or non-secure world
	 * covers it
	 */

	ret
