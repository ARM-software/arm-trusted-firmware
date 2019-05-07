/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <context.h>
#include <cortex_a76.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include <services/arm_arch_svc.h>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A76 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

#define ESR_EL3_A64_SMC0	0x5e000000
#define ESR_EL3_A32_SMC0	0x4e000000

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	/*
	 * This macro applies the mitigation for CVE-2018-3639.
	 * It implements a fast path where `SMCCC_ARCH_WORKAROUND_2`
	 * SMC calls from a lower EL running in AArch32 or AArch64
	 * will go through the fast and return early.
	 *
	 * The macro saves x2-x3 to the context. In the fast path
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
		 * EL3 via the `el3_exit` function. Otherwise NULL is
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
	 * Always enable v4 mitigation during EL3 execution. This is not
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
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639 */

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A76 Errata #1073348.
	 * This applies only to revision <= r1p0 of Cortex A76.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a76_1073348_wa
	/*
	 * Compare x0 against revision r1p0
	 */
	mov	x17, x30
	bl	check_errata_1073348
	cbz	x0, 1f
	mrs	x1, CORTEX_A76_CPUACTLR_EL1
	orr	x1, x1 ,#CORTEX_A76_CPUACTLR_EL1_DISABLE_STATIC_PREDICTION
	msr	CORTEX_A76_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a76_1073348_wa

func check_errata_1073348
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1073348

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A76 Errata #1130799.
	 * This applies only to revision <= r2p0 of Cortex A76.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a76_1130799_wa
	/*
	 * Compare x0 against revision r2p0
	 */
	mov	x17, x30
	bl	check_errata_1130799
	cbz	x0, 1f
	mrs	x1, CORTEX_A76_CPUACTLR2_EL1
	orr	x1, x1 ,#(1 << 59)
	msr	CORTEX_A76_CPUACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a76_1130799_wa

func check_errata_1130799
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_1130799

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A76 Errata #1220197.
	 * This applies only to revision <= r2p0 of Cortex A76.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a76_1220197_wa
/*
 * Compare x0 against revision r2p0
 */
	mov	x17, x30
	bl	check_errata_1220197
	cbz	x0, 1f
	mrs	x1, CORTEX_A76_CPUECTLR_EL1
	orr	x1, x1, #CORTEX_A76_CPUECTLR_EL1_WS_THR_L2
	msr	CORTEX_A76_CPUECTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a76_1220197_wa

func check_errata_1220197
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_1220197

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A76 Errata #1257314.
	 * This applies only to revision <= r3p0 of Cortex A76.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a76_1257314_wa
	/*
	 * Compare x0 against revision r3p0
	 */
	mov	x17, x30
	bl	check_errata_1257314
	cbz	x0, 1f
	mrs	x1, CORTEX_A76_CPUACTLR3_EL1
	orr	x1, x1, CORTEX_A76_CPUACTLR3_EL1_BIT_10
	msr	CORTEX_A76_CPUACTLR3_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a76_1257314_wa

func check_errata_1257314
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1257314

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A76 Errata #1262888.
	 * This applies only to revision <= r3p0 of Cortex A76.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a76_1262888_wa
	/*
	 * Compare x0 against revision r3p0
	 */
	mov	x17, x30
	bl	check_errata_1262888
	cbz	x0, 1f
	mrs	x1, CORTEX_A76_CPUECTLR_EL1
	orr	x1, x1, CORTEX_A76_CPUECTLR_EL1_BIT_51
	msr	CORTEX_A76_CPUECTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a76_1262888_wa

func check_errata_1262888
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1262888

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A76 Errata #1275112
	 * and Errata #1262606.
	 * This applies only to revision <= r3p0 of Cortex A76.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a76_1275112_1262606_wa
	/*
	 * Compare x0 against revision r3p0
	 */
	mov	x17, x30
	/*
	 * Since both errata #1275112 and #1262606 have the same check, we can
	 * invoke any one of them for the check here.
	 */
	bl	check_errata_1275112
	cbz	x0, 1f
	mrs	x1, CORTEX_A76_CPUACTLR_EL1
	orr	x1, x1, CORTEX_A76_CPUACTLR_EL1_BIT_13
	msr	CORTEX_A76_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a76_1275112_1262606_wa

func check_errata_1262606
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1262606

func check_errata_1275112
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1275112

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A76 Errata #1286807.
	 * This applies only to revision <= r3p0 of Cortex A76.
	 * Due to the nature of the errata it is applied unconditionally
	 * when built in, report it as applicable in this case
	 * ---------------------------------------------------
	 */
func check_errata_1286807
#if ERRATA_A76_1286807
	mov x0, #ERRATA_APPLIES
	ret
#else
	mov	x1, #0x30
	b	cpu_rev_var_ls
#endif
endfunc check_errata_1286807

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

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A76.
	 * Shall clobber: x0-x19
	 * -------------------------------------------------
	 */
func cortex_a76_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A76_1073348
	mov	x0, x18
	bl	errata_a76_1073348_wa
#endif

#if ERRATA_A76_1130799
	mov	x0, x18
	bl	errata_a76_1130799_wa
#endif

#if ERRATA_A76_1220197
	mov	x0, x18
	bl	errata_a76_1220197_wa
#endif

#if ERRATA_A76_1257314
	mov	x0, x18
	bl	errata_a76_1257314_wa
#endif

#if ERRATA_A76_1262606 || ERRATA_A76_1275112
	mov	x0, x18
	bl	errata_a76_1275112_1262606_wa
#endif

#if ERRATA_A76_1262888
	mov	x0, x18
	bl	errata_a76_1262888_wa
#endif

#if WORKAROUND_CVE_2018_3639
	/* If the PE implements SSBS, we don't need the dynamic workaround */
	mrs	x0, id_aa64pfr1_el1
	lsr	x0, x0, #ID_AA64PFR1_EL1_SSBS_SHIFT
	and	x0, x0, #ID_AA64PFR1_EL1_SSBS_MASK
#if !DYNAMIC_WORKAROUND_CVE_2018_3639 && ENABLE_ASSERTIONS
	cmp	x0, 0
	ASM_ASSERT(ne)
#endif
#if DYNAMIC_WORKAROUND_CVE_2018_3639
	cbnz	x0, 1f
	mrs	x0, CORTEX_A76_CPUACTLR2_EL1
	orr	x0, x0, #CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A76_CPUACTLR2_EL1, x0
	isb

#ifdef IMAGE_BL31
	/*
	 * The Cortex-A76 generic vectors are overwritten to use the vectors
	 * defined above. This is required in order to apply mitigation
	 * against CVE-2018-3639 on exception entry from lower ELs.
	 */
	adr	x0, cortex_a76_wa_cve_2018_3639_a76_vbar
	msr	vbar_el3, x0
	isb
#endif /* IMAGE_BL31 */

1:
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639 */
#endif /* WORKAROUND_CVE_2018_3639 */

#if ERRATA_DSU_798953
	bl	errata_dsu_798953_wa
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
 * Errata printing function for Cortex A76. Must follow AAPCS.
 */
func cortex_a76_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A76_1073348, cortex_a76, 1073348
	report_errata ERRATA_A76_1130799, cortex_a76, 1130799
	report_errata ERRATA_A76_1220197, cortex_a76, 1220197
	report_errata ERRATA_A76_1257314, cortex_a76, 1257314
	report_errata ERRATA_A76_1262606, cortex_a76, 1262606
	report_errata ERRATA_A76_1262888, cortex_a76, 1262888
	report_errata ERRATA_A76_1275112, cortex_a76, 1275112
	report_errata ERRATA_A76_1286807, cortex_a76, 1286807
	report_errata WORKAROUND_CVE_2018_3639, cortex_a76, cve_2018_3639
	report_errata ERRATA_DSU_798953, cortex_a76, dsu_798953
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
