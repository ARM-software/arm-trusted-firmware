/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <common/bl_common.h>
#include <common/debug.h>
#include <cortex_a57.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a57_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable all types of L2 prefetches.
	 * ---------------------------------------------
	 */
func cortex_a57_disable_l2_prefetch
	mrs	x0, CORTEX_A57_ECTLR_EL1
	orr	x0, x0, #CORTEX_A57_ECTLR_DIS_TWD_ACC_PFTCH_BIT
	mov	x1, #CORTEX_A57_ECTLR_L2_IPFTCH_DIST_MASK
	orr	x1, x1, #CORTEX_A57_ECTLR_L2_DPFTCH_DIST_MASK
	bic	x0, x0, x1
	msr	CORTEX_A57_ECTLR_EL1, x0
	isb
	dsb	ish
	ret
endfunc cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a57_disable_smp
	mrs	x0, CORTEX_A57_ECTLR_EL1
	bic	x0, x0, #CORTEX_A57_ECTLR_SMP_BIT
	msr	CORTEX_A57_ECTLR_EL1, x0
	ret
endfunc cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func cortex_a57_disable_ext_debug
	mov	x0, #1
	msr	osdlr_el1, x0
	isb
#if ERRATA_A57_817169
	/*
	 * Invalidate any TLB address
	 */
	mov	x0, #0
	tlbi	vae3, x0
#endif
	dsb	sy
	ret
endfunc cortex_a57_disable_ext_debug

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #806969.
	 * This applies only to revision r0p0 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a57_806969_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	mov	x17, x30
	bl	check_errata_806969
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_NO_ALLOC_WBWA
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a57_806969_wa

func check_errata_806969
	mov	x1, #0x00
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
	mov	x0, #ERRATA_APPLIES
	ret
endfunc check_errata_813419

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #813420.
	 * This applies only to revision r0p0 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------
	 */
func errata_a57_813420_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	mov	x17, x30
	bl	check_errata_813420
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_DCC_AS_DCCI
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a57_813420_wa

func check_errata_813420
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_813420

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #814670.
	 * This applies only to revision r0p0 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------
	 */
func errata_a57_814670_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	mov	x17, x30
	bl	check_errata_814670
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_DIS_DMB_NULLIFICATION
	msr	CORTEX_A57_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a57_814670_wa

func check_errata_814670
	mov	x1, #0x00
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
	mov	x0, #ERRATA_APPLIES
	ret
endfunc check_errata_817169

	/* --------------------------------------------------------------------
	 * Disable the over-read from the LDNP instruction.
	 *
	 * This applies to all revisions <= r1p2. The performance degradation
	 * observed with LDNP/STNP has been fixed on r1p3 and onwards.
	 *
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------------------------
	 */
func a57_disable_ldnp_overread
	/*
	 * Compare x0 against revision r1p2
	 */
	mov	x17, x30
	bl	check_errata_disable_ldnp_overread
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_DIS_OVERREAD
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc a57_disable_ldnp_overread

func check_errata_disable_ldnp_overread
	mov	x1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_disable_ldnp_overread

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #826974.
	 * This applies only to revision <= r1p1 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------
	 */
func errata_a57_826974_wa
	/*
	 * Compare x0 against revision r1p1
	 */
	mov	x17, x30
	bl	check_errata_826974
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_DIS_LOAD_PASS_DMB
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a57_826974_wa

func check_errata_826974
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_826974

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #826977.
	 * This applies only to revision <= r1p1 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------
	 */
func errata_a57_826977_wa
	/*
	 * Compare x0 against revision r1p1
	 */
	mov	x17, x30
	bl	check_errata_826977
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_GRE_NGRE_AS_NGNRE
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a57_826977_wa

func check_errata_826977
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_826977

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #828024.
	 * This applies only to revision <= r1p1 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------
	 */
func errata_a57_828024_wa
	/*
	 * Compare x0 against revision r1p1
	 */
	mov	x17, x30
	bl	check_errata_828024
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	/*
	 * Setting the relevant bits in CPUACTLR_EL1 has to be done in 2
	 * instructions here because the resulting bitmask doesn't fit in a
	 * 16-bit value so it cannot be encoded in a single instruction.
	 */
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_NO_ALLOC_WBWA
	orr	x1, x1, #(CORTEX_A57_CPUACTLR_EL1_DIS_L1_STREAMING | \
			  CORTEX_A57_CPUACTLR_EL1_DIS_STREAMING)
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a57_828024_wa

func check_errata_828024
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_828024

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #829520.
	 * This applies only to revision <= r1p2 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------
	 */
func errata_a57_829520_wa
	/*
	 * Compare x0 against revision r1p2
	 */
	mov	x17, x30
	bl	check_errata_829520
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_DIS_INDIRECT_PREDICTOR
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a57_829520_wa

func check_errata_829520
	mov	x1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_829520

	/* ---------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #833471.
	 * This applies only to revision <= r1p2 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------
	 */
func errata_a57_833471_wa
	/*
	 * Compare x0 against revision r1p2
	 */
	mov	x17, x30
	bl	check_errata_833471
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_FORCE_FPSCR_FLUSH
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a57_833471_wa

func check_errata_833471
	mov	x1, #0x12
	b	cpu_rev_var_ls
endfunc check_errata_833471

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A57 Errata #859972.
	 * This applies only to revision <= r1p3 of Cortex A57.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber:
	 * --------------------------------------------------
	 */
func errata_a57_859972_wa
	mov	x17, x30
	bl	check_errata_859972
	cbz	x0, 1f
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_DIS_INSTR_PREFETCH
	msr	CORTEX_A57_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a57_859972_wa

func check_errata_859972
	mov	x1, #0x13
	b	cpu_rev_var_ls
endfunc check_errata_859972

func check_errata_cve_2017_5715
#if WORKAROUND_CVE_2017_5715
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2017_5715

func check_errata_cve_2018_3639
#if WORKAROUND_CVE_2018_3639
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2018_3639

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A57.
	 * Shall clobber: x0-x19
	 * -------------------------------------------------
	 */
func cortex_a57_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A57_806969
	mov	x0, x18
	bl	errata_a57_806969_wa
#endif

#if ERRATA_A57_813420
	mov	x0, x18
	bl	errata_a57_813420_wa
#endif

#if ERRATA_A57_814670
	mov	x0, x18
	bl	errata_a57_814670_wa
#endif

#if A57_DISABLE_NON_TEMPORAL_HINT
	mov	x0, x18
	bl	a57_disable_ldnp_overread
#endif

#if ERRATA_A57_826974
	mov	x0, x18
	bl	errata_a57_826974_wa
#endif

#if ERRATA_A57_826977
	mov	x0, x18
	bl	errata_a57_826977_wa
#endif

#if ERRATA_A57_828024
	mov	x0, x18
	bl	errata_a57_828024_wa
#endif

#if ERRATA_A57_829520
	mov	x0, x18
	bl	errata_a57_829520_wa
#endif

#if ERRATA_A57_833471
	mov	x0, x18
	bl	errata_a57_833471_wa
#endif

#if ERRATA_A57_859972
	mov	x0, x18
	bl	errata_a57_859972_wa
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2017_5715
	adr	x0, wa_cve_2017_5715_mmu_vbar
	msr	vbar_el3, x0
	/* isb will be performed before returning from this function */
#endif

#if WORKAROUND_CVE_2018_3639
	mrs	x0, CORTEX_A57_CPUACTLR_EL1
	orr	x0, x0, #CORTEX_A57_CPUACTLR_EL1_DIS_LOAD_PASS_STORE
	msr	CORTEX_A57_CPUACTLR_EL1, x0
	isb
	dsb	sy
#endif

#if A57_ENABLE_NONCACHEABLE_LOAD_FWD
	/* ---------------------------------------------
	 * Enable higher performance non-cacheable load
	 * forwarding
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A57_CPUACTLR_EL1
	orr	x0, x0, #CORTEX_A57_CPUACTLR_EL1_EN_NC_LOAD_FWD
	msr	CORTEX_A57_CPUACTLR_EL1, x0
#endif

	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A57_ECTLR_EL1
	orr	x0, x0, #CORTEX_A57_ECTLR_SMP_BIT
	msr	CORTEX_A57_ECTLR_EL1, x0
	isb
	ret	x19
endfunc cortex_a57_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A57.
	 * ----------------------------------------------------
	 */
func cortex_a57_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

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
	bl	cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A57.
	 * -------------------------------------------------------
	 */
func cortex_a57_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

#if !SKIP_A57_L1_FLUSH_PWR_DWN
	/* -------------------------------------------------
	 * Flush the L1 caches.
	 * -------------------------------------------------
	 */
	mov	x0, #DCCISW
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
	mov	x0, #DCCISW
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
	mov	x30, x18
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A57. Must follow AAPCS.
 */
func cortex_a57_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

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

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a57_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a57 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a57_regs, "aS"
cortex_a57_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", "cpumerrsr_el1", "l2merrsr_el1", ""

func cortex_a57_cpu_reg_dump
	adr	x6, cortex_a57_regs
	mrs	x8, CORTEX_A57_ECTLR_EL1
	mrs	x9, CORTEX_A57_MERRSR_EL1
	mrs	x10, CORTEX_A57_L2MERRSR_EL1
	ret
endfunc cortex_a57_cpu_reg_dump

declare_cpu_ops_wa cortex_a57, CORTEX_A57_MIDR, \
	cortex_a57_reset_func, \
	check_errata_cve_2017_5715, \
	CPU_NO_EXTRA2_FUNC, \
	cortex_a57_core_pwr_dwn, \
	cortex_a57_cluster_pwr_dwn
