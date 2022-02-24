/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cortex_a72.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A72_BHB_LOOP_COUNT, cortex_a72
#endif /* WORKAROUND_CVE_2022_23960 */

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a72_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc cortex_a72_disable_dcache

	/* ---------------------------------------------
	 * Disable all types of L2 prefetches.
	 * ---------------------------------------------
	 */
func cortex_a72_disable_l2_prefetch
	mrs	x0, CORTEX_A72_ECTLR_EL1
	orr	x0, x0, #CORTEX_A72_ECTLR_DIS_TWD_ACC_PFTCH_BIT
	mov	x1, #CORTEX_A72_ECTLR_L2_IPFTCH_DIST_MASK
	orr	x1, x1, #CORTEX_A72_ECTLR_L2_DPFTCH_DIST_MASK
	bic	x0, x0, x1
	msr	CORTEX_A72_ECTLR_EL1, x0
	isb
	ret
endfunc cortex_a72_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable the load-store hardware prefetcher.
	 * ---------------------------------------------
	 */
func cortex_a72_disable_hw_prefetcher
	mrs	x0, CORTEX_A72_CPUACTLR_EL1
	orr	x0, x0, #CORTEX_A72_CPUACTLR_EL1_DISABLE_L1_DCACHE_HW_PFTCH
	msr	CORTEX_A72_CPUACTLR_EL1, x0
	isb
	dsb	ish
	ret
endfunc cortex_a72_disable_hw_prefetcher

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a72_disable_smp
	mrs	x0, CORTEX_A72_ECTLR_EL1
	bic	x0, x0, #CORTEX_A72_ECTLR_SMP_BIT
	msr	CORTEX_A72_ECTLR_EL1, x0
	ret
endfunc cortex_a72_disable_smp

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func cortex_a72_disable_ext_debug
	mov	x0, #1
	msr	osdlr_el1, x0
	isb
	dsb	sy
	ret
endfunc cortex_a72_disable_ext_debug

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A72 Errata #859971.
	 * This applies only to revision <= r0p3 of Cortex A72.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber:
	 * --------------------------------------------------
	 */
func errata_a72_859971_wa
	mov	x17,x30
	bl	check_errata_859971
	cbz	x0, 1f
	mrs	x1, CORTEX_A72_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A72_CPUACTLR_EL1_DIS_INSTR_PREFETCH
	msr	CORTEX_A72_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a72_859971_wa

func check_errata_859971
	mov	x1, #0x03
	b	cpu_rev_var_ls
endfunc check_errata_859971

func check_errata_cve_2017_5715
	cpu_check_csv2	x0, 1f
#if WORKAROUND_CVE_2017_5715
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
1:
	mov	x0, #ERRATA_NOT_APPLIES
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

	/* --------------------------------------------------
	 * Errata workaround for Cortex A72 Errata #1319367.
	 * This applies to all revisions of Cortex A72.
	 * --------------------------------------------------
	 */
func check_errata_1319367
#if ERRATA_A72_1319367
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_1319367

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2022_23960

func check_smccc_arch_workaround_3
	cpu_check_csv2	x0, 1f
	mov	x0, #ERRATA_APPLIES
	ret
1:
	mov	x0, #ERRATA_NOT_APPLIES
	ret
endfunc check_smccc_arch_workaround_3

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A72.
	 * -------------------------------------------------
	 */
func cortex_a72_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A72_859971
	mov	x0, x18
	bl	errata_a72_859971_wa
#endif

#if IMAGE_BL31 && (WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960)
	cpu_check_csv2	x0, 1f
	adr	x0, wa_cve_2017_5715_mmu_vbar
	msr	vbar_el3, x0
	/* isb will be performed before returning from this function */

	/* Skip CVE_2022_23960 mitigation if cve_2017_5715 mitigation applied */
	b	2f
1:
#if WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-A72 generic vectors are overridden to apply the
         * mitigation on exception entry from lower ELs for revisions >= r1p0
	 * which has CSV2 implemented.
	 */
	adr	x0, wa_cve_vbar_cortex_a72
	msr	vbar_el3, x0

	/* isb will be performed before returning from this function */
#endif /* WORKAROUND_CVE_2022_23960 */
2:
#endif /* IMAGE_BL31 &&  (WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960) */

#if WORKAROUND_CVE_2018_3639
	mrs	x0, CORTEX_A72_CPUACTLR_EL1
	orr	x0, x0, #CORTEX_A72_CPUACTLR_EL1_DIS_LOAD_PASS_STORE
	msr	CORTEX_A72_CPUACTLR_EL1, x0
	isb
	dsb	sy
#endif

	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A72_ECTLR_EL1
	orr	x0, x0, #CORTEX_A72_ECTLR_SMP_BIT
	msr	CORTEX_A72_ECTLR_EL1, x0
	isb
	ret x19
endfunc cortex_a72_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A72.
	 * ----------------------------------------------------
	 */
func cortex_a72_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_dcache

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
	mov	x0, #DCCISW
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
	mov	x30, x18
	b	cortex_a72_disable_ext_debug
endfunc cortex_a72_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A72.
	 * -------------------------------------------------------
	 */
func cortex_a72_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a72_disable_dcache

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
	bl	cortex_a72_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a72_disable_ext_debug
endfunc cortex_a72_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A72. Must follow AAPCS.
 */
func cortex_a72_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A72_859971, cortex_a72, 859971
	report_errata ERRATA_A72_1319367, cortex_a72, 1319367
	report_errata WORKAROUND_CVE_2017_5715, cortex_a72, cve_2017_5715
	report_errata WORKAROUND_CVE_2018_3639, cortex_a72, cve_2018_3639
	report_errata WORKAROUND_CVE_2022_23960, cortex_a72, cve_2022_23960

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a72_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a72 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a72_regs, "aS"
cortex_a72_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", "cpumerrsr_el1", "l2merrsr_el1", ""

func cortex_a72_cpu_reg_dump
	adr	x6, cortex_a72_regs
	mrs	x8, CORTEX_A72_ECTLR_EL1
	mrs	x9, CORTEX_A72_MERRSR_EL1
	mrs	x10, CORTEX_A72_L2MERRSR_EL1
	ret
endfunc cortex_a72_cpu_reg_dump

declare_cpu_ops_wa cortex_a72, CORTEX_A72_MIDR, \
	cortex_a72_reset_func, \
	check_errata_cve_2017_5715, \
	CPU_NO_EXTRA2_FUNC, \
	check_smccc_arch_workaround_3, \
	cortex_a72_core_pwr_dwn, \
	cortex_a72_cluster_pwr_dwn
