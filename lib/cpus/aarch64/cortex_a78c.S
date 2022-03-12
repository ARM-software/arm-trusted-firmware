/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a78c.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_a78c must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A78C_BHB_LOOP_COUNT, cortex_a78c
#endif /* WORKAROUND_CVE_2022_23960 */

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2022_23960

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A78C
	 * -------------------------------------------------
	 */
func cortex_a78c_reset_func
#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-A78c generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_cortex_a78c
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */
	isb
	ret
endfunc cortex_a78c_reset_func

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a78c_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_A78C_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A78C_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	msr	CORTEX_A78C_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a78c_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A78C. Must follow AAPCS.
 */
func cortex_a78c_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata WORKAROUND_CVE_2022_23960, cortex_a78c, cve_2022_23960

	ldp	x8, x30, [sp], #16
        ret
endfunc cortex_a78c_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a78c specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a78c_regs, "aS"
cortex_a78c_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a78c_cpu_reg_dump
	adr	x6, cortex_a78c_regs
	mrs	x8, CORTEX_A78C_CPUECTLR_EL1
	ret
endfunc cortex_a78c_cpu_reg_dump

declare_cpu_ops cortex_a78c, CORTEX_A78C_MIDR, \
	cortex_a78c_reset_func, \
	cortex_a78c_core_pwr_dwn
