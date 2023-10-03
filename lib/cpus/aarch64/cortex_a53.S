/*
 * Copyright (c) 2014-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <common/debug.h>
#include <cortex_a53.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include <lib/cpus/errata.h>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a53_disable_dcache
	sysreg_bit_clear sctlr_el3, SCTLR_C_BIT
	isb
	ret
endfunc cortex_a53_disable_dcache

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a53_disable_smp
	sysreg_bit_clear CORTEX_A53_ECTLR_EL1, CORTEX_A53_ECTLR_SMP_BIT
	isb
	dsb	sy
	ret
endfunc cortex_a53_disable_smp

/* Due to the nature of the errata it is applied unconditionally when chosen */
check_erratum_ls cortex_a53, ERRATUM(819472), CPU_REV(0, 1)
/* erratum workaround is interleaved with generic code */
add_erratum_entry cortex_a53, ERRATUM(819472), ERRATUM_ALWAYS_CHOSEN, NO_APPLY_AT_RESET

/* Due to the nature of the errata it is applied unconditionally when chosen */
check_erratum_ls cortex_a53, ERRATUM(824069), CPU_REV(0, 2)
/* erratum workaround is interleaved with generic code */
add_erratum_entry cortex_a53, ERRATUM(824069), ERRATUM_ALWAYS_CHOSEN, NO_APPLY_AT_RESET

workaround_reset_start cortex_a53, ERRATUM(826319), ERRATA_A53_826319
	mrs	x1, CORTEX_A53_L2ACTLR_EL1
	bic	x1, x1, #CORTEX_A53_L2ACTLR_ENABLE_UNIQUECLEAN
	orr	x1, x1, #CORTEX_A53_L2ACTLR_DISABLE_CLEAN_PUSH
	msr	CORTEX_A53_L2ACTLR_EL1, x1
workaround_reset_end cortex_a53, ERRATUM(826319)

check_erratum_ls cortex_a53, ERRATUM(826319), CPU_REV(0, 2)

/* Due to the nature of the errata it is applied unconditionally when chosen */
check_erratum_ls cortex_a53, ERRATUM(827319), CPU_REV(0, 2)
/* erratum workaround is interleaved with generic code */
add_erratum_entry cortex_a53, ERRATUM(827319), ERRATUM_ALWAYS_CHOSEN, NO_APPLY_AT_RESET

check_erratum_custom_start cortex_a53, ERRATUM(835769)
	cmp	x0, CPU_REV(0, 4)
	b.hi	errata_not_applies
	/*
	 * Fix potentially available for revisions r0p2, r0p3 and r0p4.
	 * If r0p2, r0p3 or r0p4; check for fix in REVIDR, else exit.
	 */
	cmp	x0, #0x01
	mov	x0, #ERRATA_APPLIES
	b.ls	exit_check_errata_835769
	/* Load REVIDR. */
	mrs	x1, revidr_el1
	/* If REVIDR[7] is set (fix exists) set ERRATA_NOT_APPLIES, else exit. */
	tbz	x1, #7, exit_check_errata_835769
errata_not_applies:
	mov	x0, #ERRATA_NOT_APPLIES
exit_check_errata_835769:
	ret
check_erratum_custom_end cortex_a53, ERRATUM(835769)

/* workaround at build time */
add_erratum_entry cortex_a53, ERRATUM(835769), ERRATA_A53_835769, NO_APPLY_AT_RESET

	/*
	 * Disable the cache non-temporal hint.
	 *
	 * This ignores the Transient allocation hint in the MAIR and treats
	 * allocations the same as non-transient allocation types. As a result,
	 * the LDNP and STNP instructions in AArch64 behave the same as the
	 * equivalent LDP and STP instructions.
	 */
workaround_reset_start cortex_a53, ERRATUM(836870), ERRATA_A53_836870 | A53_DISABLE_NON_TEMPORAL_HINT
	sysreg_bit_set CORTEX_A53_CPUACTLR_EL1, CORTEX_A53_CPUACTLR_EL1_DTAH
workaround_reset_end cortex_a53, ERRATUM(836870)

check_erratum_ls cortex_a53, ERRATUM(836870), CPU_REV(0, 3)

check_erratum_custom_start cortex_a53, ERRATUM(843419)
	mov	x1, #ERRATA_APPLIES
	mov	x2, #ERRATA_NOT_APPLIES
	cmp	x0, CPU_REV(0, 4)
	csel	x0, x1, x2, ls
	/*
	 * Fix potentially available for revision r0p4.
	 * If r0p4 check for fix in REVIDR, else exit.
	 */
	b.ne	exit_check_errata_843419
	/* Load REVIDR. */
	mrs	x3, revidr_el1
	/* If REVIDR[8] is set (fix exists) set ERRATA_NOT_APPLIES, else exit. */
	tbz	x3, #8, exit_check_errata_843419
	mov	x0, x2
exit_check_errata_843419:
	ret
check_erratum_custom_end cortex_a53, ERRATUM(843419)

/* workaround at build time */
add_erratum_entry cortex_a53, ERRATUM(843419), ERRATA_A53_843419, NO_APPLY_AT_RESET

	/*
	 * Earlier revisions of the core are affected as well, but don't
	 * have the chicken bit in the CPUACTLR register. It is expected that
	 * the rich OS takes care of that, especially as the workaround is
	 * shared with other erratas in those revisions of the CPU.
	 */
workaround_reset_start cortex_a53, ERRATUM(855873), ERRATA_A53_855873
	sysreg_bit_set CORTEX_A53_CPUACTLR_EL1, CORTEX_A53_CPUACTLR_EL1_ENDCCASCI
workaround_reset_end cortex_a53, ERRATUM(855873)

check_erratum_hs cortex_a53, ERRATUM(855873), CPU_REV(0, 3)

check_erratum_chosen cortex_a53, ERRATUM(1530924), ERRATA_A53_1530924

/* erratum has no workaround in the cpu. Generic code must take care */
add_erratum_entry cortex_a53, ERRATUM(1530924), ERRATA_A53_1530924, NO_APPLY_AT_RESET

cpu_reset_func_start cortex_a53
	/* Enable the SMP bit. */
	sysreg_bit_set CORTEX_A53_ECTLR_EL1, CORTEX_A53_ECTLR_SMP_BIT
cpu_reset_func_end cortex_a53

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

errata_report_shim cortex_a53

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
