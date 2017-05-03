/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cortex_a32.h>
#include <cpu_macros.S>


	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * Clobbers: r0-r1
	 * ---------------------------------------------
	 */
func cortex_a32_disable_smp
	ldcopr16	r0, r1, CORTEX_A32_CPUECTLR_EL1
	bic	r0, r0, #CORTEX_A32_CPUECTLR_SMPEN_BIT
	stcopr16	r0, r1, CORTEX_A32_CPUECTLR_EL1
	isb
	dsb	sy
	bx	lr
endfunc cortex_a32_disable_smp

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A32.
	 * Clobbers: r0-r1
	 * -------------------------------------------------
	 */
func cortex_a32_reset_func
	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	ldcopr16	r0, r1, CORTEX_A32_CPUECTLR_EL1
	orr	r0, r0, #CORTEX_A32_CPUECTLR_SMPEN_BIT
	stcopr16	r0, r1, CORTEX_A32_CPUECTLR_EL1
	isb
	bx	lr
endfunc cortex_a32_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A32.
	 * Clobbers: r0-r3
	 * ----------------------------------------------------
	 */
func cortex_a32_core_pwr_dwn
	/* r12 is pushed to meet the 8 byte stack alignment requirement */
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ENABLE_ASSERTIONS
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	pop	{r12, lr}
	b	cortex_a32_disable_smp
endfunc cortex_a32_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A32.
	 * Clobbers: r0-r3
	 * -------------------------------------------------------
	 */
func cortex_a32_cluster_pwr_dwn
	/* r12 is pushed to meet the 8 byte stack alignment requirement */
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ENABLE_ASSERTIONS
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif

	/* ---------------------------------------------
	 * Flush L1 cache.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* ---------------------------------------------
	 * Flush L2 cache.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	pop	{r12, lr}
	b	cortex_a32_disable_smp
endfunc cortex_a32_cluster_pwr_dwn

declare_cpu_ops cortex_a32, CORTEX_A32_MIDR, \
	cortex_a32_reset_func, \
	cortex_a32_core_pwr_dwn, \
	cortex_a32_cluster_pwr_dwn
