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

#if A53_DISABLE_NON_TEMPORAL_HINT
#undef ERRATA_A53_836870
#define ERRATA_A53_836870	1
#endif

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

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A53 Errata #826319.
	 * This applies only to revision <= r0p2 of Cortex A53.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * --------------------------------------------------
	 */
func errata_a53_826319_wa
	/*
	 * Compare r0 against revision r0p2
	 */
	mov	r2, lr
	bl	check_errata_826319
	mov	lr, r2
	cmp	r0, #ERRATA_NOT_APPLIES
	beq	1f
	ldcopr	r0, CORTEX_A53_L2ACTLR
	bic	r0, #CORTEX_A53_L2ACTLR_ENABLE_UNIQUECLEAN
	orr	r0, #CORTEX_A53_L2ACTLR_DISABLE_CLEAN_PUSH
	stcopr	r0, CORTEX_A53_L2ACTLR
1:
	bx	lr
endfunc errata_a53_826319_wa

func check_errata_826319
	mov	r1, #0x02
	b	cpu_rev_var_ls
endfunc check_errata_826319

	/* ---------------------------------------------------------------------
	 * Disable the cache non-temporal hint.
	 *
	 * This ignores the Transient allocation hint in the MAIR and treats
	 * allocations the same as non-transient allocation types. As a result,
	 * the LDNP and STNP instructions in AArch64 behave the same as the
	 * equivalent LDP and STP instructions.
	 *
	 * This is relevant only for revisions <= r0p3 of Cortex-A53.
	 * From r0p4 and onwards, the bit to disable the hint is enabled by
	 * default at reset.
	 *
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------------------------
	 */
func a53_disable_non_temporal_hint
	/*
	 * Compare r0 against revision r0p3
	 */
	mov		r2, lr
	bl		check_errata_disable_non_temporal_hint
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A53_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A53_CPUACTLR_DTAH
	stcopr16	r0, r1, CORTEX_A53_CPUACTLR
1:
	bx		lr
endfunc a53_disable_non_temporal_hint

func check_errata_disable_non_temporal_hint
	mov	r1, #0x03
	b	cpu_rev_var_ls
endfunc check_errata_disable_non_temporal_hint

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A53 Errata #855873.
	 *
	 * This applies only to revisions >= r0p3 of Cortex A53.
	 * Earlier revisions of the core are affected as well, but don't
	 * have the chicken bit in the CPUACTLR register. It is expected that
	 * the rich OS takes care of that, especially as the workaround is
	 * shared with other erratas in those revisions of the CPU.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * --------------------------------------------------
	 */
func errata_a53_855873_wa
	/*
	 * Compare r0 against revision r0p3 and higher
	 */
	mov		r2, lr
	bl		check_errata_855873
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A53_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A53_CPUACTLR_ENDCCASCI
	stcopr16	r0, r1, CORTEX_A53_CPUACTLR
1:
	bx		lr
endfunc errata_a53_855873_wa

func check_errata_855873
	mov	r1, #0x03
	b	cpu_rev_var_hs
endfunc check_errata_855873

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A53.
	 * Shall clobber: r0-r6
	 * -------------------------------------------------
	 */
func cortex_a53_reset_func
	mov	r5, lr
	bl	cpu_get_rev_var
	mov	r4, r0

#if ERRATA_A53_826319
	mov	r0, r4
	bl	errata_a53_826319_wa
#endif

#if ERRATA_A53_836870
	mov	r0, r4
	bl	a53_disable_non_temporal_hint
#endif

#if ERRATA_A53_855873
	mov	r0, r4
	bl	errata_a53_855873_wa
#endif

	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	ldcopr16	r0, r1, CORTEX_A53_ECTLR
	orr64_imm	r0, r1, CORTEX_A53_ECTLR_SMP_BIT
	stcopr16	r0, r1,	CORTEX_A53_ECTLR
	isb
	bx	r5
endfunc cortex_a53_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A53.
	 * ----------------------------------------------------
	 */
func cortex_a53_core_pwr_dwn
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

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A53. Must follow AAPCS.
 */
func cortex_a53_errata_report
	push	{r12, lr}

	bl	cpu_get_rev_var
	mov	r4, r0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A53_826319, cortex_a53, 826319
	report_errata ERRATA_A53_836870, cortex_a53, disable_non_temporal_hint
	report_errata ERRATA_A53_855873, cortex_a53, 855873

	pop	{r12, lr}
	bx	lr
endfunc cortex_a53_errata_report
#endif

declare_cpu_ops cortex_a53, CORTEX_A53_MIDR, \
	cortex_a53_reset_func, \
	cortex_a53_core_pwr_dwn, \
	cortex_a53_cluster_pwr_dwn
