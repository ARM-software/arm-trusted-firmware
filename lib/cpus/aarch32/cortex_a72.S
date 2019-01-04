/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <common/debug.h>
#include <cortex_a72.h>
#include <cpu_macros.S>

	/* ---------------------------------------------
	 * Disable all types of L2 prefetches.
	 * ---------------------------------------------
	 */
func cortex_a72_disable_l2_prefetch
	ldcopr16	r0, r1, CORTEX_A72_ECTLR
	orr64_imm	r0, r1, CORTEX_A72_ECTLR_DIS_TWD_ACC_PFTCH_BIT
	bic64_imm	r0, r1, (CORTEX_A72_ECTLR_L2_IPFTCH_DIST_MASK | \
				CORTEX_A72_ECTLR_L2_DPFTCH_DIST_MASK)
	stcopr16	r0, r1, CORTEX_A72_ECTLR
	isb
	bx	lr
endfunc cortex_a72_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable the load-store hardware prefetcher.
	 * ---------------------------------------------
	 */
func cortex_a72_disable_hw_prefetcher
	ldcopr16	r0, r1, CORTEX_A72_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A72_CPUACTLR_DISABLE_L1_DCACHE_HW_PFTCH
	stcopr16	r0, r1, CORTEX_A72_CPUACTLR
	isb
	dsb	ish
	bx	lr
endfunc cortex_a72_disable_hw_prefetcher

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * Clobbers: r0-r1
	 * ---------------------------------------------
	 */
func cortex_a72_disable_smp
	ldcopr16	r0, r1, CORTEX_A72_ECTLR
	bic64_imm	r0, r1, CORTEX_A72_ECTLR_SMP_BIT
	stcopr16	r0, r1, CORTEX_A72_ECTLR
	bx	lr
endfunc cortex_a72_disable_smp

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func cortex_a72_disable_ext_debug
	mov	r0, #1
	stcopr	r0, DBGOSDLR
	isb
	dsb	sy
	bx	lr
endfunc cortex_a72_disable_ext_debug

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A72 Errata #859971.
	 * This applies only to revision <= r0p3 of Cortex A72.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a72_859971_wa
	mov		r2,lr
	bl		check_errata_859971
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A72_CPUACTLR
	orr64_imm	r1, r1, CORTEX_A72_CPUACTLR_DIS_INSTR_PREFETCH
	stcopr16	r0, r1, CORTEX_A72_CPUACTLR
1:
	bx   		lr
endfunc errata_a72_859971_wa

func check_errata_859971
	mov		r1, #0x03
	b		cpu_rev_var_ls
endfunc check_errata_859971

func check_errata_cve_2017_5715
	mov	r0, #ERRATA_MISSING
	bx	lr
endfunc check_errata_cve_2017_5715

func check_errata_cve_2018_3639
#if WORKAROUND_CVE_2018_3639
	mov	r0, #ERRATA_APPLIES
#else
	mov	r0, #ERRATA_MISSING
#endif
	bx	lr
endfunc check_errata_cve_2018_3639

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A72.
	 * -------------------------------------------------
	 */
func cortex_a72_reset_func
	mov	r5, lr
	bl	cpu_get_rev_var
	mov	r4, r0

#if ERRATA_A72_859971
	mov	r0, r4
	bl	errata_a72_859971_wa
#endif

#if WORKAROUND_CVE_2018_3639
	ldcopr16	r0, r1, CORTEX_A72_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A72_CPUACTLR_DIS_LOAD_PASS_STORE
	stcopr16	r0, r1, CORTEX_A72_CPUACTLR
	isb
	dsb	sy
#endif

	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	ldcopr16	r0, r1, CORTEX_A72_ECTLR
	orr64_imm	r0, r1, CORTEX_A72_ECTLR_SMP_BIT
	stcopr16	r0, r1,	CORTEX_A72_ECTLR
	isb
	bx	r5
endfunc cortex_a72_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A72.
	 * ----------------------------------------------------
	 */
func cortex_a72_core_pwr_dwn
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ENABLE_ASSERTIONS
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable the load-store hardware prefetcher.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_hw_prefetcher

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
	bl	cortex_a72_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	pop	{r12, lr}
	b	cortex_a72_disable_ext_debug
endfunc cortex_a72_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A72.
	 * -------------------------------------------------------
	 */
func cortex_a72_cluster_pwr_dwn
	push	{r12, lr}

	/* Assert if cache is enabled */
#if ENABLE_ASSERTIONS
	ldcopr	r0, SCTLR
	tst	r0, #SCTLR_C_BIT
	ASM_ASSERT(eq)
#endif

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable the load-store hardware prefetcher.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_hw_prefetcher

#if !SKIP_A72_L1_FLUSH_PWR_DWN
	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level1
#endif

	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* -------------------------------------------------
	 * Flush the L2 caches.
	 * -------------------------------------------------
	 */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	pop	{r12, lr}
	b	cortex_a72_disable_ext_debug
endfunc cortex_a72_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A72. Must follow AAPCS.
 */
func cortex_a72_errata_report
	push	{r12, lr}

	bl	cpu_get_rev_var
	mov	r4, r0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A72_859971, cortex_a72, 859971
	report_errata WORKAROUND_CVE_2017_5715, cortex_a72, cve_2017_5715
	report_errata WORKAROUND_CVE_2018_3639, cortex_a72, cve_2018_3639

	pop	{r12, lr}
	bx	lr
endfunc cortex_a72_errata_report
#endif

declare_cpu_ops cortex_a72, CORTEX_A72_MIDR, \
	cortex_a72_reset_func, \
	cortex_a72_core_pwr_dwn, \
	cortex_a72_cluster_pwr_dwn
