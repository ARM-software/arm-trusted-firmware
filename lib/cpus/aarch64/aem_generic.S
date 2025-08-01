/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <aem_generic.h>
#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>

cpu_reset_prologue aem_generic

func aem_generic_core_pwr_dwn
	/* ---------------------------------------------
	 * AEM model supports L3 caches in which case L2
	 * will be private per core caches and flush
	 * from L1 to L2 is not sufficient.
	 * ---------------------------------------------
	 */
	mrs	x1, clidr_el1

	/* ---------------------------------------------
	 * Check if L3 cache is implemented.
	 * ---------------------------------------------
	 */
	tst	x1, ((1 << CLIDR_FIELD_WIDTH) - 1) << CTYPE_SHIFT(3)

	/* ---------------------------------------------
	 * There is no L3 cache, flush L1 to L2 only.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	b.eq	dcsw_op_level1

	mov	x18, x30

	/* ---------------------------------------------
	 * Flush L1 cache to L2.
	 * ---------------------------------------------
	 */
	bl	dcsw_op_level1
	mov	x30, x18

	/* ---------------------------------------------
	 * Flush L2 cache to L3.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	b	dcsw_op_level2
endfunc aem_generic_core_pwr_dwn

func aem_generic_cluster_pwr_dwn
	/* ---------------------------------------------
	 * Flush all caches to PoC.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	b	dcsw_op_all
endfunc aem_generic_cluster_pwr_dwn

cpu_reset_func_start aem_generic
cpu_reset_func_end aem_generic

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
declare_cpu_ops aem_generic, BASE_AEM_MIDR, aem_generic_reset_func, \
	aem_generic_core_pwr_dwn, \
	aem_generic_cluster_pwr_dwn

/* cpu_ops for Foundation FVP */
declare_cpu_ops aem_generic, FOUNDATION_AEM_MIDR, aem_generic_reset_func, \
	aem_generic_core_pwr_dwn, \
	aem_generic_cluster_pwr_dwn
