/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
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

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A35.
	 * Clobbers: x0
	 * -------------------------------------------------
	 */
func cortex_a35_reset_func
	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A35_CPUECTLR_EL1
	orr	x0, x0, #CORTEX_A35_CPUECTLR_SMPEN_BIT
	msr	CORTEX_A35_CPUECTLR_EL1, x0
	isb
	ret
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
