/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a35.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a35_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc cortex_a35_disable_dcache

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a35_disable_smp
	mrs	x0, CORTEX_A35_CPUECTLR_EL1
	bic	x0, x0, #CORTEX_A35_CPUECTLR_SMPEN_BIT
	msr	CORTEX_A35_CPUECTLR_EL1, x0
	isb
	dsb	sy
	ret
endfunc cortex_a35_disable_smp

	 /* ---------------------------------------------------
	 * Errata Workaround for Cortex A35 Errata #855472.
	 * This applies to revisions r0p0 of Cortex A35.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------
	 */
func errata_a35_855472_wa
	 /*
	  * Compare x0 against revision r0p0
	  */
	 mov	x17, x30
	 bl	check_errata_855472
	 cbz	x0, 1f
	 mrs	x1, CORTEX_A35_CPUACTLR_EL1
	 orr	x1, x1, #CORTEX_A35_CPUACTLR_EL1_ENDCCASCI
	 msr	CORTEX_A35_CPUACTLR_EL1, x1
	 isb
1:
	ret	x17
endfunc errata_a35_855472_wa

func check_errata_855472
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_855472

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A35.
	 * Clobbers: x0
	 * -------------------------------------------------
	 */
func cortex_a35_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var

#if ERRATA_A35_855472
	bl	errata_a35_855472_wa
#endif

	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A35_CPUECTLR_EL1
	orr	x0, x0, #CORTEX_A35_CPUECTLR_SMPEN_BIT
	msr	CORTEX_A35_CPUECTLR_EL1, x0
	isb
	ret	x19
endfunc cortex_a35_reset_func

func cortex_a35_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a35_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a35_disable_smp
endfunc cortex_a35_core_pwr_dwn

func cortex_a35_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a35_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* ---------------------------------------------
	 * Flush L2 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a35_disable_smp
endfunc cortex_a35_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A35. Must follow AAPCS.
 */
func cortex_a35_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A35_855472, cortex_a35, 855472

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a35_errata_report
#endif


	/* ---------------------------------------------
	 * This function provides cortex_a35 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a35_regs, "aS"
cortex_a35_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a35_cpu_reg_dump
	adr	x6, cortex_a35_regs
	mrs	x8, CORTEX_A35_CPUECTLR_EL1
	ret
endfunc cortex_a35_cpu_reg_dump

declare_cpu_ops cortex_a35, CORTEX_A35_MIDR, \
	cortex_a35_reset_func, \
	cortex_a35_core_pwr_dwn, \
	cortex_a35_cluster_pwr_dwn
