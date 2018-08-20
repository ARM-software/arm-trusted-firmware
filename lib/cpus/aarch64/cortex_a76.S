/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arm_arch_svc.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <context.h>
#include <cortex_a76.h>
#include <cpu_macros.S>
#include <plat_macros.S>

#if !DYNAMIC_WORKAROUND_CVE_2018_3639
#error Cortex A76 requires DYNAMIC_WORKAROUND_CVE_2018_3639=1
#endif

#define ESR_EL3_A64_SMC0	0x5e000000
#define ESR_EL3_A32_SMC0	0x4e000000

	/*
	 * This macro applies the mitigation for CVE-2018-3639.
	 * It implements a fash path where `SMCCC_ARCH_WORKAROUND_2`
	 * SMC calls from a lower EL running in AArch32 or AArch64
	 * will go through the fast and return early.
	 *
	 * The macro saves x2-x3 to the context.  In the fast path
	 * x0-x3 registers do not need to be restored as the calling
	 * context will have saved them.
	 */
	.macro apply_cve_2018_3639_wa _is_sync_exception _esr_el3_val
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]

	.if \_is_sync_exception
		/*
		 * Ensure SMC is coming from A64/A32 state on #0
		 * with W0 = SMCCC_ARCH_WORKAROUND_2
		 *
		 * This sequence evaluates as:
		 *    (W0==SMCCC_ARCH_WORKAROUND_2) ? (ESR_EL3==SMC#0) : (NE)
		 * allowing use of a single branch operation
		 */
		orr	w2, wzr, #SMCCC_ARCH_WORKAROUND_2
		cmp	x0, x2
		mrs	x3, esr_el3
		mov_imm	w2, \_esr_el3_val
		ccmp	w2, w3, #0, eq
		/*
		 * Static predictor will predict a fall-through, optimizing
		 * the `SMCCC_ARCH_WORKAROUND_2` fast path.
		 */
		bne	1f

		/*
		 * The sequence below implements the `SMCCC_ARCH_WORKAROUND_2`
		 * fast path.
		 */
		cmp	x1, xzr /* enable/disable check */

		/*
		 * When the calling context wants mitigation disabled,
		 * we program the mitigation disable function in the
		 * CPU context, which gets invoked on subsequent exits from
		 * EL3 via the `el3_exit` function.  Otherwise NULL is
		 * programmed in the CPU context, which results in caller's
		 * inheriting the EL3 mitigation state (enabled) on subsequent
		 * `el3_exit`.
		 */
		mov	x0, xzr
		adr	x1, cortex_a76_disable_wa_cve_2018_3639
		csel	x1, x1, x0, eq
		str	x1, [sp, #CTX_CVE_2018_3639_OFFSET + CTX_CVE_2018_3639_DISABLE]

		mrs	x2, CORTEX_A76_CPUACTLR2_EL1
		orr	x1, x2, #CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE
		bic	x3, x2, #CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE
		csel	x3, x3, x1, eq
		msr	CORTEX_A76_CPUACTLR2_EL1, x3
		eret	/* ERET implies ISB */
	.endif
1:
	/*
	 * Always enable v4 mitigation during EL3 execution.  This is not
	 * required for the fast path above because it does not perform any
	 * memory loads.
	 */
	mrs	x2, CORTEX_A76_CPUACTLR2_EL1
	orr	x2, x2, #CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A76_CPUACTLR2_EL1, x2
	isb

	/*
	 * The caller may have passed arguments to EL3 via x2-x3.
	 * Restore these registers from the context before jumping to the
	 * main runtime vector table entry.
	 */
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	.endm

vector_base cortex_a76_wa_cve_2018_3639_a76_vbar

	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a76_sync_exception_sp_el0
	b	sync_exception_sp_el0
end_vector_entry cortex_a76_sync_exception_sp_el0

vector_entry cortex_a76_irq_sp_el0
	b	irq_sp_el0
end_vector_entry cortex_a76_irq_sp_el0

vector_entry cortex_a76_fiq_sp_el0
	b	fiq_sp_el0
end_vector_entry cortex_a76_fiq_sp_el0

vector_entry cortex_a76_serror_sp_el0
	b	serror_sp_el0
end_vector_entry cortex_a76_serror_sp_el0

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a76_sync_exception_sp_elx
	b	sync_exception_sp_elx
end_vector_entry cortex_a76_sync_exception_sp_elx

vector_entry cortex_a76_irq_sp_elx
	b	irq_sp_elx
end_vector_entry cortex_a76_irq_sp_elx

vector_entry cortex_a76_fiq_sp_elx
	b	fiq_sp_elx
end_vector_entry cortex_a76_fiq_sp_elx

vector_entry cortex_a76_serror_sp_elx
	b	serror_sp_elx
end_vector_entry cortex_a76_serror_sp_elx

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a76_sync_exception_aarch64
	apply_cve_2018_3639_wa _is_sync_exception=1 _esr_el3_val=ESR_EL3_A64_SMC0
	b	sync_exception_aarch64
end_vector_entry cortex_a76_sync_exception_aarch64

vector_entry cortex_a76_irq_aarch64
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	irq_aarch64
end_vector_entry cortex_a76_irq_aarch64

vector_entry cortex_a76_fiq_aarch64
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	fiq_aarch64
end_vector_entry cortex_a76_fiq_aarch64

vector_entry cortex_a76_serror_aarch64
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
	b	serror_aarch64
end_vector_entry cortex_a76_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a76_sync_exception_aarch32
	apply_cve_2018_3639_wa _is_sync_exception=1 _esr_el3_val=ESR_EL3_A32_SMC0
	b	sync_exception_aarch32
end_vector_entry cortex_a76_sync_exception_aarch32

vector_entry cortex_a76_irq_aarch32
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	irq_aarch32
end_vector_entry cortex_a76_irq_aarch32

vector_entry cortex_a76_fiq_aarch32
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	fiq_aarch32
end_vector_entry cortex_a76_fiq_aarch32

vector_entry cortex_a76_serror_aarch32
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
	b	serror_aarch32
end_vector_entry cortex_a76_serror_aarch32

func check_errata_cve_2018_3639
#if WORKAROUND_CVE_2018_3639
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2018_3639

func cortex_a76_disable_wa_cve_2018_3639
	mrs	x0, CORTEX_A76_CPUACTLR2_EL1
	bic	x0, x0, #CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A76_CPUACTLR2_EL1, x0
	isb
	ret
endfunc cortex_a76_disable_wa_cve_2018_3639

func cortex_a76_reset_func
	mov	x19, x30
#if WORKAROUND_CVE_2018_3639
	mrs	x0, CORTEX_A76_CPUACTLR2_EL1
	orr	x0, x0, #CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A76_CPUACTLR2_EL1, x0
	isb
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2018_3639
	/*
	 * The Cortex-A76 generic vectors are overwritten to use the vectors
	 * defined above.  This is required in order to apply mitigation
	 * against CVE-2018-3639 on exception entry from lower ELs.
	 */
	adr	x0, cortex_a76_wa_cve_2018_3639_a76_vbar
	msr	vbar_el3, x0
	isb
#endif

#if ERRATA_DSU_936184
	bl	errata_dsu_936184_wa
#endif
	ret	x19
endfunc cortex_a76_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a76_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A76_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A76_CORE_PWRDN_EN_MASK
	msr	CORTEX_A76_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a76_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex Cortex A76. Must follow AAPCS.
 */
func cortex_a76_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata WORKAROUND_CVE_2018_3639, cortex_a76, cve_2018_3639
	report_errata ERRATA_DSU_936184, cortex_a76, dsu_936184

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a76_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a76 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a76_regs, "aS"
cortex_a76_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a76_cpu_reg_dump
	adr	x6, cortex_a76_regs
	mrs	x8, CORTEX_A76_CPUECTLR_EL1
	ret
endfunc cortex_a76_cpu_reg_dump

declare_cpu_ops_wa cortex_a76, CORTEX_A76_MIDR, \
	cortex_a76_reset_func, \
	CPU_NO_EXTRA1_FUNC, \
	cortex_a76_disable_wa_cve_2018_3639, \
	cortex_a76_core_pwr_dwn
