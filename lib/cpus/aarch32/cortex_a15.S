/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cortex_a15.h>
#include <cpu_macros.S>

/*
 * Cortex-A15 support LPAE and Virtualization Extensions.
 * Don't care if confiugration uses or not LPAE and VE.
 * Therefore, where we don't check ARCH_IS_ARMV7_WITH_LPAE/VE
 */

	.macro assert_cache_enabled
#if ENABLE_ASSERTIONS
		ldcopr	r0, SCTLR
		tst	r0, #SCTLR_C_BIT
		ASM_ASSERT(eq)
#endif
	.endm

func cortex_a15_disable_smp
	ldcopr	r0, ACTLR
	bic	r0, #CORTEX_A15_ACTLR_SMP_BIT
	stcopr	r0, ACTLR
	isb
#if ERRATA_A15_816470
	/*
	 * Invalidate any TLB address
	 */
	mov	r0, #0
	stcopr	r0, TLBIMVA
#endif
	dsb	sy
	bx	lr
endfunc cortex_a15_disable_smp

func cortex_a15_enable_smp
	ldcopr	r0, ACTLR
	orr	r0, #CORTEX_A15_ACTLR_SMP_BIT
	stcopr	r0, ACTLR
	isb
	bx	lr
endfunc cortex_a15_enable_smp

	/* ----------------------------------------------------
	 * Errata Workaround for Cortex A15 Errata #816470.
	 * This applies only to revision >= r3p0 of Cortex A15.
	 * ----------------------------------------------------
	 */
func check_errata_816470
	/*
	 * Even though this is only needed for revision >= r3p0, it is always
	 * applied because of the low cost of the workaround.
	 */
	mov	r0, #ERRATA_APPLIES
	bx	lr
endfunc check_errata_816470

	/* ----------------------------------------------------
	 * Errata Workaround for Cortex A15 Errata #827671.
	 * This applies only to revision >= r3p0 of Cortex A15.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ----------------------------------------------------
	 */
func errata_a15_827671_wa
	/*
	 * Compare r0 against revision r3p0
	 */
	mov	r2, lr
	bl	check_errata_827671
	cmp	r0, #ERRATA_NOT_APPLIES
	beq	1f
	ldcopr	r0, CORTEX_A15_ACTLR2
	orr	r0, #CORTEX_A15_ACTLR2_INV_DCC_BIT
	stcopr	r0, CORTEX_A15_ACTLR2
	isb
1:
	bx	r2
endfunc errata_a15_827671_wa

func check_errata_827671
	mov	r1, #0x30
	b	cpu_rev_var_hs
endfunc check_errata_827671

func check_errata_cve_2017_5715
#if WORKAROUND_CVE_2017_5715
	mov	r0, #ERRATA_APPLIES
#else
	mov	r0, #ERRATA_MISSING
#endif
	bx	lr
endfunc check_errata_cve_2017_5715

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A15. Must follow AAPCS.
 */
func cortex_a15_errata_report
	push	{r12, lr}

	bl	cpu_get_rev_var
	mov	r4, r0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A15_816470, cortex_a15, 816470
	report_errata ERRATA_A15_827671, cortex_a15, 827671
	report_errata WORKAROUND_CVE_2017_5715, cortex_a15, cve_2017_5715

	pop	{r12, lr}
	bx	lr
endfunc cortex_a15_errata_report
#endif

func cortex_a15_reset_func
	mov	r5, lr
	bl	cpu_get_rev_var

#if ERRATA_A15_827671
	bl	errata_a15_827671_wa
#endif

#if IMAGE_BL32 && WORKAROUND_CVE_2017_5715
	ldcopr	r0, ACTLR
	orr	r0, #CORTEX_A15_ACTLR_INV_BTB_BIT
	stcopr	r0, ACTLR
	ldr	r0, =workaround_icache_inv_runtime_exceptions
	stcopr	r0, VBAR
	stcopr	r0, MVBAR
	/* isb will be applied in the course of the reset func */
#endif

	mov	lr, r5
	b	cortex_a15_enable_smp
endfunc cortex_a15_reset_func

func cortex_a15_core_pwr_dwn
	push	{r12, lr}

	assert_cache_enabled

	/* Flush L1 cache */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level1

	/* Exit cluster coherency */
	pop	{r12, lr}
	b	cortex_a15_disable_smp
endfunc cortex_a15_core_pwr_dwn

func cortex_a15_cluster_pwr_dwn
	push	{r12, lr}

	assert_cache_enabled

	/* Flush L1 caches */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level1

	bl	plat_disable_acp

	/* Exit cluster coherency */
	pop	{r12, lr}
	b	cortex_a15_disable_smp
endfunc cortex_a15_cluster_pwr_dwn

declare_cpu_ops cortex_a15, CORTEX_A15_MIDR, \
	cortex_a15_reset_func, \
	cortex_a15_core_pwr_dwn, \
	cortex_a15_cluster_pwr_dwn
