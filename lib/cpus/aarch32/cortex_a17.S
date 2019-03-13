/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cortex_a17.h>
#include <cpu_macros.S>

	.macro assert_cache_enabled
#if ENABLE_ASSERTIONS
		ldcopr	r0, SCTLR
		tst	r0, #SCTLR_C_BIT
		ASM_ASSERT(eq)
#endif
	.endm

func cortex_a17_disable_smp
	ldcopr	r0, ACTLR
	bic	r0, #CORTEX_A17_ACTLR_SMP_BIT
	stcopr	r0, ACTLR
	isb
	dsb	sy
	bx	lr
endfunc cortex_a17_disable_smp

func cortex_a17_enable_smp
	ldcopr	r0, ACTLR
	orr	r0, #CORTEX_A17_ACTLR_SMP_BIT
	stcopr	r0, ACTLR
	isb
	bx	lr
endfunc cortex_a17_enable_smp

	/* ----------------------------------------------------
	 * Errata Workaround for Cortex A17 Errata #852421.
	 * This applies only to revision <= r1p2 of Cortex A17.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ----------------------------------------------------
	 */
func errata_a17_852421_wa
	/*
	 * Compare r0 against revision r1p2
	 */
	mov	r2, lr
	bl	check_errata_852421
	cmp	r0, #ERRATA_NOT_APPLIES
	beq	1f
	ldcopr	r0, CORTEX_A17_IMP_DEF_REG1
	orr	r0, r0, #(1<<24)
	stcopr	r0, CORTEX_A17_IMP_DEF_REG1
1:
	bx	r2
endfunc errata_a17_852421_wa

func check_errata_852421
	mov	r1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_852421

	/* ----------------------------------------------------
	 * Errata Workaround for Cortex A17 Errata #852423.
	 * This applies only to revision <= r1p2 of Cortex A17.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ----------------------------------------------------
	 */
func errata_a17_852423_wa
	/*
	 * Compare r0 against revision r1p2
	 */
	mov	r2, lr
	bl	check_errata_852423
	cmp	r0, #ERRATA_NOT_APPLIES
	beq	1f
	ldcopr	r0, CORTEX_A17_IMP_DEF_REG1
	orr	r0, r0, #(1<<12)
	stcopr	r0, CORTEX_A17_IMP_DEF_REG1
1:
	bx	r2
endfunc errata_a17_852423_wa

func check_errata_852423
	mov	r1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_852423

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
 * Errata printing function for Cortex A17. Must follow AAPCS.
 */
func cortex_a17_errata_report
	push	{r12, lr}

	bl	cpu_get_rev_var
	mov	r4, r0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A17_852421, cortex_a17, 852421
	report_errata ERRATA_A17_852423, cortex_a17, 852423
	report_errata WORKAROUND_CVE_2017_5715, cortex_a17, cve_2017_5715

	pop	{r12, lr}
	bx	lr
endfunc cortex_a17_errata_report
#endif

func cortex_a17_reset_func
	mov	r5, lr
	bl	cpu_get_rev_var
	mov	r4, r0

#if ERRATA_A17_852421
	mov	r0, r4
	bl	errata_a17_852421_wa
#endif

#if ERRATA_A17_852423
	mov	r0, r4
	bl	errata_a17_852423_wa
#endif

#if IMAGE_BL32 && WORKAROUND_CVE_2017_5715
	ldr	r0, =workaround_bpiall_runtime_exceptions
	stcopr	r0, VBAR
	stcopr	r0, MVBAR
	/* isb will be applied in the course of the reset func */
#endif

	mov	lr, r5
	b	cortex_a17_enable_smp
endfunc cortex_a17_reset_func

func cortex_a17_core_pwr_dwn
	push	{r12, lr}

	assert_cache_enabled

	/* Flush L1 cache */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level1

	/* Exit cluster coherency */
	pop	{r12, lr}
	b	cortex_a17_disable_smp
endfunc cortex_a17_core_pwr_dwn

func cortex_a17_cluster_pwr_dwn
	push	{r12, lr}

	assert_cache_enabled

	/* Flush L1 caches */
	mov	r0, #DC_OP_CISW
	bl	dcsw_op_level1

	bl	plat_disable_acp

	/* Exit cluster coherency */
	pop	{r12, lr}
	b	cortex_a17_disable_smp
endfunc cortex_a17_cluster_pwr_dwn

declare_cpu_ops cortex_a17, CORTEX_A17_MIDR, \
	cortex_a17_reset_func, \
	cortex_a17_core_pwr_dwn, \
	cortex_a17_cluster_pwr_dwn
