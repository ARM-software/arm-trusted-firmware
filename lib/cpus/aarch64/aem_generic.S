/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <aem_generic.h>
#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>

func aem_generic_core_pwr_dwn
	/* ---------------------------------------------
	 * Disable the Data Cache.
	 * ---------------------------------------------
	 */
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb

	mov	x0, #DCCISW

	/* ---------------------------------------------
	 * Flush L1 cache to PoU.
	 * ---------------------------------------------
	 */
	b	dcsw_op_louis
endfunc aem_generic_core_pwr_dwn


func aem_generic_cluster_pwr_dwn
	/* ---------------------------------------------
	 * Disable the Data Cache.
	 * ---------------------------------------------
	 */
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb

	/* ---------------------------------------------
	 * Flush L1 and L2 caches to PoC.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	b	dcsw_op_all
endfunc aem_generic_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for AEM. Must follow AAPCS.
 */
func aem_generic_errata_report
	ret
endfunc aem_generic_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cpu specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.aem_generic_regs, "aS"
aem_generic_regs:  /* The ascii list of register names to be reported */
	.asciz	"" /* no registers to report */

func aem_generic_cpu_reg_dump
	adr	x6, aem_generic_regs
	ret
endfunc aem_generic_cpu_reg_dump


/* cpu_ops for Base AEM FVP */
declare_cpu_ops aem_generic, BASE_AEM_MIDR, CPU_NO_RESET_FUNC, \
	aem_generic_core_pwr_dwn, \
	aem_generic_cluster_pwr_dwn

/* cpu_ops for Foundation FVP */
declare_cpu_ops aem_generic, FOUNDATION_AEM_MIDR, CPU_NO_RESET_FUNC, \
	aem_generic_core_pwr_dwn, \
	aem_generic_cluster_pwr_dwn
