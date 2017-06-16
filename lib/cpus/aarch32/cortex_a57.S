/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cortex_a57.h>
#include <cpu_macros.S>
#include <debug.h>

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * Clobbers: r0-r1
	 * ---------------------------------------------
	 */
func cortex_a57_disable_smp
	ldcopr16	r0, r1, CORTEX_A57_ECTLR
	bic64_imm	r0, r1, CORTEX_A57_ECTLR_SMP_BIT
	stcopr16	r0, r1, CORTEX_A57_ECTLR
	bx	lr
endfunc cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Disable all types of L2 prefetches.
	 * Clobbers: r0-r2
	 * ---------------------------------------------
	 */
func cortex_a57_disable_l2_prefetch
	ldcopr16	r0, r1, CORTEX_A57_ECTLR
	orr64_imm	r0, r1, CORTEX_A57_ECTLR_DIS_TWD_ACC_PFTCH_BIT
	bic64_imm	r0, r1, (CORTEX_A57_ECTLR_L2_IPFTCH_DIST_MASK | \
				 CORTEX_A57_ECTLR_L2_DPFTCH_DIST_MASK)
	stcopr16	r0, r1, CORTEX_A57_ECTLR
	isb
	dsb	ish
	bx	lr
endfunc cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func cortex_a57_disable_ext_debug
	mov	r0, #1
	stcopr	r0, DBGOSDLR
	isb
	dsb	sy
	bx	lr
endfunc cortex_a57_disable_ext_debug

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A57.
	 * -------------------------------------------------
	 */
func cortex_a57_reset_func
	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	ldcopr16	r0, r1, CORTEX_A57_ECTLR
	orr64_imm	r0, r1, CORTEX_A57_ECTLR_SMP_BIT
	stcopr16	r0, r1,	CORTEX_A57_ECTLR
	isb
	bx	lr
endfunc cortex_a57_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A57.
	 * ----------------------------------------------------
	 */
func cortex_a57_core_pwr_dwn
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ASM_ASSERTION
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

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
	bl	cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	pop	{r12, lr}
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A57.
	 * Clobbers: r0-r3
	 * -------------------------------------------------------
	 */
func cortex_a57_cluster_pwr_dwn
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ASM_ASSERTION
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Flush L1 caches.
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
	 * Flush L2 caches.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	pop	{r12, lr}
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_cluster_pwr_dwn

declare_cpu_ops cortex_a57, CORTEX_A57_MIDR, \
	cortex_a57_reset_func, \
	cortex_a57_core_pwr_dwn, \
	cortex_a57_cluster_pwr_dwn
