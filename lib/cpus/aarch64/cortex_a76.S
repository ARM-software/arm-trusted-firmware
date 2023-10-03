/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a76.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include <services/arm_arch_svc.h>
#include "wa_cve_2022_23960_bhb.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A76 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif
	.globl cortex_a76_reset_func
	.globl cortex_a76_core_pwr_dwn
	.globl cortex_a76_disable_wa_cve_2018_3639

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A76 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
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
	 * context will have saved them. The macro also saves
	 * x29-x30 to the context in the sync_exception path.
	 */
	.macro apply_cve_2018_3639_wa _is_sync_exception _esr_el3_val
	stp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	.if \_is_sync_exception
	stp	x29, x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
	mov_imm	w2, \_esr_el3_val
	bl	apply_cve_2018_3639_sync_wa
	ldp	x29, x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
	.endif
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
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639 || WORKAROUND_CVE_2022_23960
vector_base cortex_a76_wa_cve_vbar

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

#if WORKAROUND_CVE_2022_23960
	apply_cve_2022_23960_bhb_wa CORTEX_A76_BHB_LOOP_COUNT
#endif /* WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	apply_cve_2018_3639_wa _is_sync_exception=1 _esr_el3_val=ESR_EL3_A64_SMC0
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639*/

	b	sync_exception_aarch64
end_vector_entry cortex_a76_sync_exception_aarch64

vector_entry cortex_a76_irq_aarch64

#if WORKAROUND_CVE_2022_23960
	apply_cve_2022_23960_bhb_wa CORTEX_A76_BHB_LOOP_COUNT
#endif /* WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639*/

	b	irq_aarch64
end_vector_entry cortex_a76_irq_aarch64

vector_entry cortex_a76_fiq_aarch64

#if WORKAROUND_CVE_2022_23960
	apply_cve_2022_23960_bhb_wa CORTEX_A76_BHB_LOOP_COUNT
#endif /* WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639*/

	b	fiq_aarch64
end_vector_entry cortex_a76_fiq_aarch64

vector_entry cortex_a76_serror_aarch64

#if WORKAROUND_CVE_2022_23960
	apply_cve_2022_23960_bhb_wa CORTEX_A76_BHB_LOOP_COUNT
#endif /* WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A64_SMC0
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639*/

	b	serror_aarch64
end_vector_entry cortex_a76_serror_aarch64

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry cortex_a76_sync_exception_aarch32

#if WORKAROUND_CVE_2022_23960
	apply_cve_2022_23960_bhb_wa CORTEX_A76_BHB_LOOP_COUNT
#endif /* WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	apply_cve_2018_3639_wa _is_sync_exception=1 _esr_el3_val=ESR_EL3_A32_SMC0
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639*/

	b	sync_exception_aarch32
end_vector_entry cortex_a76_sync_exception_aarch32

vector_entry cortex_a76_irq_aarch32

#if WORKAROUND_CVE_2022_23960
	apply_cve_2022_23960_bhb_wa CORTEX_A76_BHB_LOOP_COUNT
#endif /* WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639*/

	b	irq_aarch32
end_vector_entry cortex_a76_irq_aarch32

vector_entry cortex_a76_fiq_aarch32

#if WORKAROUND_CVE_2022_23960
	apply_cve_2022_23960_bhb_wa CORTEX_A76_BHB_LOOP_COUNT
#endif /* WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639*/

	b	fiq_aarch32
end_vector_entry cortex_a76_fiq_aarch32

vector_entry cortex_a76_serror_aarch32

#if WORKAROUND_CVE_2022_23960
	apply_cve_2022_23960_bhb_wa CORTEX_A76_BHB_LOOP_COUNT
#endif /* WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	apply_cve_2018_3639_wa _is_sync_exception=0 _esr_el3_val=ESR_EL3_A32_SMC0
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639*/

	b	serror_aarch32
end_vector_entry cortex_a76_serror_aarch32
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639 || WORKAROUND_CVE_2022_23960 */

#if DYNAMIC_WORKAROUND_CVE_2018_3639
	/*
	 * -----------------------------------------------------------------
	 * This function applies the mitigation for CVE-2018-3639
	 * specifically for sync exceptions. It implements a fast path
	 * where `SMCCC_ARCH_WORKAROUND_2` SMC calls from a lower EL
	 * running in AArch64 will go through the fast and return early.
	 *
	 * In the fast path x0-x3 registers do not need to be restored as the
	 * calling context will have saved them.
	 *
	 * Caller must pass value of esr_el3 to compare via x2.
	 * Save and restore these registers outside of this function from the
	 * context before jumping to the main runtime vector table entry.
	 *
	 * Shall clobber: x0-x3, x30
	 * -----------------------------------------------------------------
	 */
func apply_cve_2018_3639_sync_wa
	/*
	 * Ensure SMC is coming from A64/A32 state on #0
	 * with W0 = SMCCC_ARCH_WORKAROUND_2
	 *
	 * This sequence evaluates as:
	 *    (W0==SMCCC_ARCH_WORKAROUND_2) ? (ESR_EL3==SMC#0) : (NE)
	 * allowing use of a single branch operation
	 * X2 populated outside this function with the SMC FID.
	 */
	orr	w3, wzr, #SMCCC_ARCH_WORKAROUND_2
	cmp	x0, x3
	mrs	x3, esr_el3

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
	ldp	x29, x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X29]
	/*
	* `SMCCC_ARCH_WORKAROUND_2`fast path return to lower EL.
	*/
	exception_return /* exception_return contains ISB */
1:
	ret
endfunc apply_cve_2018_3639_sync_wa
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639 */

workaround_reset_start cortex_a76, ERRATUM(1073348), ERRATA_A76_1073348
	sysreg_bit_set CORTEX_A76_CPUACTLR_EL1 ,CORTEX_A76_CPUACTLR_EL1_DISABLE_STATIC_PREDICTION
workaround_reset_end cortex_a76, ERRATUM(1073348)

check_erratum_ls cortex_a76, ERRATUM(1073348), CPU_REV(1, 0)

workaround_reset_start cortex_a76, ERRATUM(1130799), ERRATA_A76_1130799
	sysreg_bit_set CORTEX_A76_CPUACTLR2_EL1, CORTEX_A76_CPUACTLR2_EL1_BIT_59
	msr	CORTEX_A76_CPUACTLR2_EL1, x1
workaround_reset_end cortex_a76, ERRATUM(1130799)

check_erratum_ls cortex_a76, ERRATUM(1130799), CPU_REV(2, 0)

workaround_reset_start cortex_a76, ERRATUM(1220197), ERRATA_A76_1220197
	sysreg_bit_set CORTEX_A76_CPUECTLR_EL1, CORTEX_A76_CPUECTLR_EL1_WS_THR_L2
workaround_reset_end cortex_a76, ERRATUM(1220197)

check_erratum_ls cortex_a76, ERRATUM(1220197), CPU_REV(2, 0)

workaround_reset_start cortex_a76, ERRATUM(1257314), ERRATA_A76_1257314
	sysreg_bit_set CORTEX_A76_CPUACTLR3_EL1, CORTEX_A76_CPUACTLR3_EL1_BIT_10
workaround_reset_end cortex_a76, ERRATUM(1257314)

check_erratum_ls cortex_a76, ERRATUM(1257314), CPU_REV(3, 0)

workaround_reset_start cortex_a76, ERRATUM(1262606), ERRATA_A76_1262606
	sysreg_bit_set CORTEX_A76_CPUACTLR_EL1, CORTEX_A76_CPUACTLR_EL1_BIT_13
workaround_reset_end cortex_a76, ERRATUM(1262606)

check_erratum_ls cortex_a76, ERRATUM(1262606), CPU_REV(3, 0)

workaround_reset_start cortex_a76, ERRATUM(1262888), ERRATA_A76_1262888
	sysreg_bit_set CORTEX_A76_CPUECTLR_EL1, CORTEX_A76_CPUECTLR_EL1_BIT_51
workaround_reset_end cortex_a76, ERRATUM(1262888)

check_erratum_ls cortex_a76, ERRATUM(1262888), CPU_REV(3, 0)

workaround_reset_start cortex_a76, ERRATUM(1275112), ERRATA_A76_1275112
	sysreg_bit_set CORTEX_A76_CPUACTLR_EL1, CORTEX_A76_CPUACTLR_EL1_BIT_13
workaround_reset_end cortex_a76, ERRATUM(1275112)

check_erratum_ls cortex_a76, ERRATUM(1275112), CPU_REV(3, 0)

check_erratum_custom_start cortex_a76, ERRATUM(1286807)
#if ERRATA_A76_1286807
	mov x0, #ERRATA_APPLIES
	ret
#else
	mov	x1, #0x30
	b	cpu_rev_var_ls
#endif
check_erratum_custom_end cortex_a76, ERRATUM(1286807)

workaround_reset_start cortex_a76, ERRATUM(1791580), ERRATA_A76_1791580
	sysreg_bit_set CORTEX_A76_CPUACTLR2_EL1, CORTEX_A76_CPUACTLR2_EL1_BIT_2
workaround_reset_end cortex_a76, ERRATUM(1791580)

check_erratum_ls cortex_a76, ERRATUM(1791580), CPU_REV(4, 0)

workaround_reset_start cortex_a76, ERRATUM(1868343), ERRATA_A76_1868343
	sysreg_bit_set CORTEX_A76_CPUACTLR_EL1, CORTEX_A76_CPUACTLR_EL1_BIT_13
workaround_reset_end cortex_a76, ERRATUM(1868343)

check_erratum_ls cortex_a76, ERRATUM(1868343), CPU_REV(4, 0)

workaround_reset_start cortex_a76, ERRATUM(1946160), ERRATA_A76_1946160
	mov	x0, #3
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3900002
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0

	mov	x0, #4
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3800082
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0

	mov	x0, #5
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3800200
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF003E0
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0
workaround_reset_end cortex_a76, ERRATUM(1946160)

check_erratum_range cortex_a76, ERRATUM(1946160), CPU_REV(3, 0), CPU_REV(4, 1)

workaround_runtime_start cortex_a76, ERRATUM(2743102), ERRATA_A76_2743102
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end cortex_a76, ERRATUM(2743102)

check_erratum_ls cortex_a76, ERRATUM(2743102), CPU_REV(4, 1)

check_erratum_chosen cortex_a76, CVE(2018, 3639), WORKAROUND_CVE_2018_3639

func cortex_a76_disable_wa_cve_2018_3639
	sysreg_bit_clear CORTEX_A76_CPUACTLR2_EL1, CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE
	isb
	ret
endfunc cortex_a76_disable_wa_cve_2018_3639

/* --------------------------------------------------------------
 * Errata Workaround for Cortex A76 Errata #1165522.
 * This applies only to revisions <= r3p0 of Cortex A76.
 * Due to the nature of the errata it is applied unconditionally
 * when built in, report it as applicable in this case
 * --------------------------------------------------------------
 */
check_erratum_custom_start cortex_a76, ERRATUM(1165522)
#if ERRATA_A76_1165522
	mov	x0, #ERRATA_APPLIES
	ret
#else
	mov	x1, #0x30
	b	cpu_rev_var_ls
#endif
check_erratum_custom_end cortex_a76, ERRATUM(1165522)

check_erratum_chosen cortex_a76, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

/* erratum has no workaround in the cpu. Generic code must take care */
add_erratum_entry cortex_a76, CVE(2022, 23960), WORKAROUND_CVE_2022_23960, NO_APPLY_AT_RESET

/* ERRATA_DSU_798953 :
 * The errata is defined in dsu_helpers.S but applies to cortex_a76
 * as well. Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_cortex_a76_798953, check_errata_dsu_798953
.equ erratum_cortex_a76_798953_wa, errata_dsu_798953_wa
add_erratum_entry cortex_a76, ERRATUM(798953), ERRATA_DSU_798953, APPLY_AT_RESET

/* ERRATA_DSU_936184 :
 * The errata is defined in dsu_helpers.S but applies to cortex_a76
 * as well. Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_cortex_a76_936184, check_errata_dsu_936184
.equ erratum_cortex_a76_936184_wa, errata_dsu_936184_wa
add_erratum_entry cortex_a76, ERRATUM(936184), ERRATA_DSU_936184, APPLY_AT_RESET

cpu_reset_func_start cortex_a76

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
	sysreg_bit_set CORTEX_A76_CPUACTLR2_EL1, CORTEX_A76_CPUACTLR2_EL1_DISABLE_LOAD_PASS_STORE
	isb

#ifdef IMAGE_BL31
	/*
	 * The Cortex-A76 generic vectors are overwritten to use the vectors
	 * defined above. This is required in order to apply mitigation
	 * against CVE-2018-3639 on exception entry from lower ELs.
	 * If the below vector table is used, skip overriding it again for
	 *  CVE_2022_23960 as both use the same vbar.
	 */
	override_vector_table cortex_a76_wa_cve_vbar
	isb
	b	2f
#endif /* IMAGE_BL31 */

1:
#endif /* DYNAMIC_WORKAROUND_CVE_2018_3639 */
#endif /* WORKAROUND_CVE_2018_3639 */

#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-A76 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs. This will be bypassed
	 * if DYNAMIC_WORKAROUND_CVE_2018_3639 has overridden the vectors.
	 */
	override_vector_table cortex_a76_wa_cve_vbar
	isb
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */
2:
cpu_reset_func_end cortex_a76

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a76_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set CORTEX_A76_CPUPWRCTLR_EL1, CORTEX_A76_CORE_PWRDN_EN_MASK

	apply_erratum cortex_a76, ERRATUM(2743102), ERRATA_A76_2743102

	isb
	ret
endfunc cortex_a76_core_pwr_dwn

errata_report_shim cortex_a76

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
	CPU_NO_EXTRA3_FUNC, \
	cortex_a76_core_pwr_dwn
