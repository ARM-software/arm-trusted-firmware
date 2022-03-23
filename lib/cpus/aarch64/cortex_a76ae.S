/*
 * Copyright (c) 2019-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a76ae.h>
#include <cpu_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A76AE must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A76AE supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A76AE_BHB_LOOP_COUNT, cortex_a76ae
#endif /* WORKAROUND_CVE_2022_23960 */

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif /* WORKAROUND_CVE_2022_23960 */
	ret
endfunc check_errata_cve_2022_23960

	/* --------------------------------------------
	 * The CPU Ops reset function for Cortex-A76AE.
	 * Shall clobber: x0-x19
	 * --------------------------------------------
	 */
func cortex_a76ae_reset_func
#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-A76ae generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_cortex_a76ae
	msr	vbar_el3, x0
	isb
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */

	ret
endfunc cortex_a76ae_reset_func

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a76ae_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_A76AE_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A76AE_CORE_PWRDN_EN_MASK
	msr	CORTEX_A76AE_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a76ae_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex-A76AE. Must follow AAPCS.
 */
func cortex_a76ae_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata WORKAROUND_CVE_2022_23960, cortex_a76ae, cve_2022_23960

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a76ae_errata_report
#endif	/* REPORT_ERRATA */

	/* ---------------------------------------------
	 * This function provides cortex_a76ae specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a76ae_regs, "aS"
cortex_a76ae_regs:  /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a76ae_cpu_reg_dump
	adr	x6, cortex_a76ae_regs
	mrs	x8, CORTEX_A76AE_CPUECTLR_EL1
	ret
endfunc cortex_a76ae_cpu_reg_dump

declare_cpu_ops cortex_a76ae, CORTEX_A76AE_MIDR, cortex_a76ae_reset_func, \
	cortex_a76ae_core_pwr_dwn
