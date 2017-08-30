/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cortex_a53.h>
#include <cpu_macros.S>
#include <debug.h>
#include <plat_macros.S>

#if A53_DISABLE_NON_TEMPORAL_HINT
#undef ERRATA_A53_836870
#define ERRATA_A53_836870	1
#endif

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a53_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc cortex_a53_disable_dcache

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a53_disable_smp
	mrs	x0, CORTEX_A53_ECTLR_EL1
	bic	x0, x0, #CORTEX_A53_ECTLR_SMP_BIT
	msr	CORTEX_A53_ECTLR_EL1, x0
	isb
	dsb	sy
	ret
endfunc cortex_a53_disable_smp

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A53 Errata #826319.
	 * This applies only to revision <= r0p2 of Cortex A53.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a53_826319_wa
	/*
	 * Compare x0 against revision r0p2
	 */
	mov	x17, x30
	bl	check_errata_826319
	cbz	x0, 1f
	mrs	x1, CORTEX_A53_L2ACTLR_EL1
	bic	x1, x1, #CORTEX_A53_L2ACTLR_ENABLE_UNIQUECLEAN
	orr	x1, x1, #CORTEX_A53_L2ACTLR_DISABLE_CLEAN_PUSH
	msr	CORTEX_A53_L2ACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a53_826319_wa

func check_errata_826319
	mov	x1, #0x02
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
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ---------------------------------------------------------------------
	 */
func a53_disable_non_temporal_hint
	/*
	 * Compare x0 against revision r0p3
	 */
	mov	x17, x30
	bl	check_errata_disable_non_temporal_hint
	cbz	x0, 1f
	mrs	x1, CORTEX_A53_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A53_CPUACTLR_EL1_DTAH
	msr	CORTEX_A53_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc a53_disable_non_temporal_hint

func check_errata_disable_non_temporal_hint
	mov	x1, #0x03
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
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a53_855873_wa
	/*
	 * Compare x0 against revision r0p3 and higher
	 */
        mov     x17, x30
        bl      check_errata_855873
        cbz     x0, 1f

	mrs	x1, CORTEX_A53_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A53_CPUACTLR_EL1_ENDCCASCI
	msr	CORTEX_A53_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a53_855873_wa

func check_errata_855873
	mov	x1, #0x03
	b	cpu_rev_var_hs
endfunc check_errata_855873

/*
 * Errata workaround for Cortex A53 Errata #835769.
 * This applies to revisions <= r0p4 of Cortex A53.
 * This workaround is statically enabled at build time.
 */
func check_errata_835769
	mov	x1, #0x04
	b	cpu_rev_var_ls
endfunc check_errata_835769

/*
 * Errata workaround for Cortex A53 Errata #843419.
 * This applies to revisions <= r0p4 of Cortex A53.
 * This workaround is statically enabled at build time.
 */
func check_errata_843419
	mov	x1, #0x04
	b	cpu_rev_var_ls
endfunc check_errata_843419

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A53.
	 * Shall clobber: x0-x19
	 * -------------------------------------------------
	 */
func cortex_a53_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0


#if ERRATA_A53_826319
	mov	x0, x18
	bl	errata_a53_826319_wa
#endif

#if ERRATA_A53_836870
	mov	x0, x18
	bl	a53_disable_non_temporal_hint
#endif

#if ERRATA_A53_855873
	mov	x0, x18
	bl	errata_a53_855873_wa
#endif

	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A53_ECTLR_EL1
	orr	x0, x0, #CORTEX_A53_ECTLR_SMP_BIT
	msr	CORTEX_A53_ECTLR_EL1, x0
	isb
	ret	x19
endfunc cortex_a53_reset_func

func cortex_a53_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a53_disable_dcache

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
	mov	x30, x18
	b	cortex_a53_disable_smp
endfunc cortex_a53_core_pwr_dwn

func cortex_a53_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a53_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
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
	mov	x0, #DCCISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a53_disable_smp
endfunc cortex_a53_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A53. Must follow AAPCS.
 */
func cortex_a53_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A53_826319, cortex_a53, 826319
	report_errata ERRATA_A53_835769, cortex_a53, 835769
	report_errata ERRATA_A53_836870, cortex_a53, disable_non_temporal_hint
	report_errata ERRATA_A53_843419, cortex_a53, 843419
	report_errata ERRATA_A53_855873, cortex_a53, 855873

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a53_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a53 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a53_regs, "aS"
cortex_a53_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", "cpumerrsr_el1", "l2merrsr_el1", \
		"cpuactlr_el1", ""

func cortex_a53_cpu_reg_dump
	adr	x6, cortex_a53_regs
	mrs	x8, CORTEX_A53_ECTLR_EL1
	mrs	x9, CORTEX_A53_MERRSR_EL1
	mrs	x10, CORTEX_A53_L2MERRSR_EL1
	mrs	x11, CORTEX_A53_CPUACTLR_EL1
	ret
endfunc cortex_a53_cpu_reg_dump

declare_cpu_ops cortex_a53, CORTEX_A53_MIDR, \
	cortex_a53_reset_func, \
	cortex_a53_core_pwr_dwn, \
	cortex_a53_cluster_pwr_dwn
