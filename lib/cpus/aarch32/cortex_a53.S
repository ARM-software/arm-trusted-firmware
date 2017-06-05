/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cortex_a53.h>
#include <cpu_macros.S>
#include <debug.h>

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a53_disable_smp
	ldcopr16	r0, r1, CORTEX_A53_ECTLR
	bic64_imm	r0, r1, CORTEX_A53_ECTLR_SMP_BIT
	stcopr16	r0, r1, CORTEX_A53_ECTLR
	isb
	dsb	sy
	bx	lr
endfunc cortex_a53_disable_smp

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A53.
	 * -------------------------------------------------
	 */
func cortex_a53_reset_func
	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	ldcopr16	r0, r1, CORTEX_A53_ECTLR
	orr64_imm	r0, r1, CORTEX_A53_ECTLR_SMP_BIT
	stcopr16	r0, r1,	CORTEX_A53_ECTLR
	isb
	bx	lr
endfunc cortex_a53_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A53.
	 * ----------------------------------------------------
	 */
func cortex_a53_core_pwr_dwn
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ASM_ASSERTION
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
	b	cortex_a53_disable_smp
endfunc cortex_a53_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A53.
	 * Clobbers: r0-r3
	 * -------------------------------------------------------
	 */
func cortex_a53_cluster_pwr_dwn
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ASM_ASSERTION
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
	pop	{r12, lr}
	b	cortex_a53_disable_smp
endfunc cortex_a53_cluster_pwr_dwn

declare_cpu_ops cortex_a53, CORTEX_A53_MIDR, \
	cortex_a53_reset_func, \
	cortex_a53_core_pwr_dwn, \
	cortex_a53_cluster_pwr_dwn
