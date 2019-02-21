/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <common/debug.h>
#include <cortex_a57.h>
#include <cpu_macros.S>

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
#if ERRATA_A57_817169
	/*
	 * Invalidate any TLB address
	 */
	mov	r0, #0
	stcopr	r0, TLBIMVA
#endif
	dsb	sy
	bx	lr
endfunc cortex_a57_disable_ext_debug

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #806969.
	 * This applies only to revision r0p0 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * --------------------------------------------------
	 */
func errata_a57_806969_wa
	/*
	 * Compare r0 against revision r0p0
	 */
	mov		r2, lr
	bl		check_errata_806969
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_NO_ALLOC_WBWA
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx	lr
endfunc errata_a57_806969_wa

func check_errata_806969
	mov	r1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_806969

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #813419.
	 * This applies only to revision r0p0 of Cortex A57.
	 * ---------------------------------------------------
	 */
func check_errata_813419
	/*
	 * Even though this is only needed for revision r0p0, it
	 * is always applied due to limitations of the current
	 * errata framework.
	 */
	mov	r0, #ERRATA_APPLIES
	bx	lr
endfunc check_errata_813419

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #813420.
	 * This applies only to revision r0p0 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a57_813420_wa
	/*
	 * Compare r0 against revision r0p0
	 */
	mov		r2, lr
	bl		check_errata_813420
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_DCC_AS_DCCI
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx		lr
endfunc errata_a57_813420_wa

func check_errata_813420
	mov	r1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_813420

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #814670.
	 * This applies only to revision r0p0 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a57_814670_wa
	/*
	 * Compare r0 against revision r0p0
	 */
	mov		r2, lr
	bl		check_errata_814670
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_DIS_DMB_NULLIFICATION
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
	isb
1:
	bx		r2
endfunc errata_a57_814670_wa

func check_errata_814670
	mov	r1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_814670

	/* ----------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #817169.
	 * This applies only to revision <= r0p1 of Cortex A57.
	 * ----------------------------------------------------
	 */
func check_errata_817169
	/*
	 * Even though this is only needed for revision <= r0p1, it
	 * is always applied because of the low cost of the workaround.
	 */
	mov	r0, #ERRATA_APPLIES
	bx	lr
endfunc check_errata_817169

	/* --------------------------------------------------------------------
	 * Disable the over-read from the LDNP instruction.
	 *
	 * This applies to all revisions <= r1p2. The performance degradation
	 * observed with LDNP/STNP has been fixed on r1p3 and onwards.
	 *
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------------------------
	 */
func a57_disable_ldnp_overread
	/*
	 * Compare r0 against revision r1p2
	 */
	mov		r2, lr
	bl		check_errata_disable_ldnp_overread
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_DIS_OVERREAD
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx		lr
endfunc a57_disable_ldnp_overread

func check_errata_disable_ldnp_overread
	mov	r1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_disable_ldnp_overread

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #826974.
	 * This applies only to revision <= r1p1 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a57_826974_wa
	/*
	 * Compare r0 against revision r1p1
	 */
	mov		r2, lr
	bl		check_errata_826974
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_DIS_LOAD_PASS_DMB
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx		lr
endfunc errata_a57_826974_wa

func check_errata_826974
	mov	r1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_826974

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #826977.
	 * This applies only to revision <= r1p1 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a57_826977_wa
	/*
	 * Compare r0 against revision r1p1
	 */
	mov		r2, lr
	bl		check_errata_826977
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_GRE_NGRE_AS_NGNRE
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx		lr
endfunc errata_a57_826977_wa

func check_errata_826977
	mov	r1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_826977

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #828024.
	 * This applies only to revision <= r1p1 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a57_828024_wa
	/*
	 * Compare r0 against revision r1p1
	 */
	mov		r2, lr
	bl		check_errata_828024
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	/*
	 * Setting the relevant bits in CORTEX_A57_CPUACTLR has to be done in 2
	 * instructions here because the resulting bitmask doesn't fit in a
	 * 16-bit value so it cannot be encoded in a single instruction.
	 */
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_NO_ALLOC_WBWA
	orr64_imm	r0, r1, (CORTEX_A57_CPUACTLR_DIS_L1_STREAMING | CORTEX_A57_CPUACTLR_DIS_STREAMING)
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx		lr
endfunc errata_a57_828024_wa

func check_errata_828024
	mov	r1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_828024

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #829520.
	 * This applies only to revision <= r1p2 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a57_829520_wa
	/*
	 * Compare r0 against revision r1p2
	 */
	mov		r2, lr
	bl		check_errata_829520
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_DIS_INDIRECT_PREDICTOR
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx		lr
endfunc errata_a57_829520_wa

func check_errata_829520
	mov	r1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_829520

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #833471.
	 * This applies only to revision <= r1p2 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a57_833471_wa
	/*
	 * Compare r0 against revision r1p2
	 */
	mov		r2, lr
	bl		check_errata_833471
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r1, r1, CORTEX_A57_CPUACTLR_FORCE_FPSCR_FLUSH
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx		lr
endfunc errata_a57_833471_wa

func check_errata_833471
	mov	r1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_833471

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #859972.
	 * This applies only to revision <= r1p3 of Cortex A57.
	 * Inputs:
	 * r0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: r0-r3
	 * ---------------------------------------------------
	 */
func errata_a57_859972_wa
	mov		r2, lr
	bl		check_errata_859972
	mov		lr, r2
	cmp		r0, #ERRATA_NOT_APPLIES
	beq		1f
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r1, r1, CORTEX_A57_CPUACTLR_DIS_INSTR_PREFETCH
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
1:
	bx		lr
endfunc errata_a57_859972_wa

func check_errata_859972
	mov	r1, #0x13
	b	cpu_rev_var_ls
endfunc check_errata_859972

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
	 * The CPU Ops reset function for Cortex-A57.
	 * Shall clobber: r0-r6
	 * -------------------------------------------------
	 */
func cortex_a57_reset_func
	mov	r5, lr
	bl	cpu_get_rev_var
	mov	r4, r0

#if ERRATA_A57_806969
	mov	r0, r4
	bl	errata_a57_806969_wa
#endif

#if ERRATA_A57_813420
	mov	r0, r4
	bl	errata_a57_813420_wa
#endif

#if ERRATA_A57_814670
	mov	r0, r4
	bl	errata_a57_814670_wa
#endif

#if A57_DISABLE_NON_TEMPORAL_HINT
	mov	r0, r4
	bl	a57_disable_ldnp_overread
#endif

#if ERRATA_A57_826974
	mov	r0, r4
	bl	errata_a57_826974_wa
#endif

#if ERRATA_A57_826977
	mov	r0, r4
	bl	errata_a57_826977_wa
#endif

#if ERRATA_A57_828024
	mov	r0, r4
	bl	errata_a57_828024_wa
#endif

#if ERRATA_A57_829520
	mov	r0, r4
	bl	errata_a57_829520_wa
#endif

#if ERRATA_A57_833471
	mov	r0, r4
	bl	errata_a57_833471_wa
#endif

#if ERRATA_A57_859972
	mov	r0, r4
	bl	errata_a57_859972_wa
#endif

#if WORKAROUND_CVE_2018_3639
	ldcopr16	r0, r1, CORTEX_A57_CPUACTLR
	orr64_imm	r0, r1, CORTEX_A57_CPUACTLR_DIS_LOAD_PASS_STORE
	stcopr16	r0, r1, CORTEX_A57_CPUACTLR
	isb
	dsb	sy
#endif

	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	ldcopr16	r0, r1, CORTEX_A57_ECTLR
	orr64_imm	r0, r1, CORTEX_A57_ECTLR_SMP_BIT
	stcopr16	r0, r1,	CORTEX_A57_ECTLR
	isb
	bx	r5
endfunc cortex_a57_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A57.
	 * ----------------------------------------------------
	 */
func cortex_a57_core_pwr_dwn
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
#if ENABLE_ASSERTIONS
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

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A57. Must follow AAPCS.
 */
func cortex_a57_errata_report
	push	{r12, lr}

	bl	cpu_get_rev_var
	mov	r4, r0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A57_806969, cortex_a57, 806969
	report_errata ERRATA_A57_813419, cortex_a57, 813419
	report_errata ERRATA_A57_813420, cortex_a57, 813420
	report_errata ERRATA_A57_814670, cortex_a57, 814670
	report_errata ERRATA_A57_817169, cortex_a57, 817169
	report_errata A57_DISABLE_NON_TEMPORAL_HINT, cortex_a57, \
		disable_ldnp_overread
	report_errata ERRATA_A57_826974, cortex_a57, 826974
	report_errata ERRATA_A57_826977, cortex_a57, 826977
	report_errata ERRATA_A57_828024, cortex_a57, 828024
	report_errata ERRATA_A57_829520, cortex_a57, 829520
	report_errata ERRATA_A57_833471, cortex_a57, 833471
	report_errata ERRATA_A57_859972, cortex_a57, 859972
	report_errata WORKAROUND_CVE_2017_5715, cortex_a57, cve_2017_5715
	report_errata WORKAROUND_CVE_2018_3639, cortex_a57, cve_2018_3639

	pop	{r12, lr}
	bx	lr
endfunc cortex_a57_errata_report
#endif

declare_cpu_ops cortex_a57, CORTEX_A57_MIDR, \
	cortex_a57_reset_func, \
	cortex_a57_core_pwr_dwn, \
	cortex_a57_cluster_pwr_dwn
