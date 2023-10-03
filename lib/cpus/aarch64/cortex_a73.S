/*
 * Copyright (c) 2016-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a73.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache
	 * ---------------------------------------------
	 */
func cortex_a73_disable_dcache
	sysreg_bit_clear sctlr_el3, SCTLR_C_BIT
	isb
	ret
endfunc cortex_a73_disable_dcache

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a73_disable_smp
	sysreg_bit_clear CORTEX_A73_CPUECTLR_EL1, CORTEX_A73_CPUECTLR_SMP_BIT
	isb
	dsb	sy
	ret
endfunc cortex_a73_disable_smp

func check_smccc_arch_workaround_3
	mov	x0, #ERRATA_APPLIES
	ret
endfunc check_smccc_arch_workaround_3

workaround_reset_start cortex_a73, ERRATUM(852427), ERRATA_A73_852427
	sysreg_bit_set CORTEX_A73_DIAGNOSTIC_REGISTER, BIT(12)
workaround_reset_end cortex_a73, ERRATUM(852427)

check_erratum_ls cortex_a73, ERRATUM(852427), CPU_REV(0, 0)

workaround_reset_start cortex_a73, ERRATUM(855423), ERRATA_A73_855423
	sysreg_bit_set CORTEX_A73_IMP_DEF_REG2, BIT(7)
workaround_reset_end cortex_a73, ERRATUM(855423)

check_erratum_ls cortex_a73, ERRATUM(855423), CPU_REV(0, 1)

workaround_reset_start cortex_a73, CVE(2017, 5715), WORKAROUND_CVE_2017_5715
#if IMAGE_BL31
	override_vector_table wa_cve_2017_5715_bpiall_vbar
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a73, CVE(2017, 5715)

check_erratum_custom_start cortex_a73, CVE(2017, 5715)
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
check_erratum_custom_end cortex_a73, CVE(2017, 5715)

workaround_reset_start cortex_a73, CVE(2018, 3639), WORKAROUND_CVE_2018_3639
	sysreg_bit_set CORTEX_A73_IMP_DEF_REG1, CORTEX_A73_IMP_DEF_REG1_DISABLE_LOAD_PASS_STORE
workaround_reset_end cortex_a73, CVE(2018, 3639)

check_erratum_chosen cortex_a73, CVE(2018, 3639), WORKAROUND_CVE_2018_3639

workaround_reset_start cortex_a73, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/* Skip installing vector table again for CVE_2022_23960 */
	adr	x0, wa_cve_2017_5715_bpiall_vbar
	mrs	x1, vbar_el3

	cmp	x0, x1
	b.eq	1f
	msr     vbar_el3, x0
1:
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a73, CVE(2022, 23960)

check_erratum_custom_start cortex_a73, CVE(2022, 23960)
#if WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960
	cpu_check_csv2  x0, 1f
	mov	x0, #ERRATA_APPLIES
	ret
 1:
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif /* WORKAROUND_CVE_2022_23960 */
	ret
#endif /* WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960 */
	mov	x0, #ERRATA_MISSING
	ret
check_erratum_custom_end cortex_a73, CVE(2022, 23960)

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A73.
	 * -------------------------------------------------
	 */

cpu_reset_func_start cortex_a73
	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * Clobbers : x0
	 * ---------------------------------------------
	 */
	sysreg_bit_set CORTEX_A73_CPUECTLR_EL1, CORTEX_A73_CPUECTLR_SMP_BIT
cpu_reset_func_end cortex_a73

func cortex_a73_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a73_disable_dcache

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
	b	cortex_a73_disable_smp
endfunc cortex_a73_core_pwr_dwn

func cortex_a73_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a73_disable_dcache

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
	b	cortex_a73_disable_smp
endfunc cortex_a73_cluster_pwr_dwn


errata_report_shim cortex_a73

	/* ---------------------------------------------
	 * This function provides cortex_a73 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a73_regs, "aS"
cortex_a73_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", "l2merrsr_el1", ""

func cortex_a73_cpu_reg_dump
	adr	x6, cortex_a73_regs
	mrs	x8, CORTEX_A73_CPUECTLR_EL1
	mrs	x9, CORTEX_A73_L2MERRSR_EL1
	ret
endfunc cortex_a73_cpu_reg_dump

declare_cpu_ops_wa cortex_a73, CORTEX_A73_MIDR, \
	cortex_a73_reset_func, \
	check_erratum_cortex_a73_5715, \
	CPU_NO_EXTRA2_FUNC, \
	check_smccc_arch_workaround_3, \
	cortex_a73_core_pwr_dwn, \
	cortex_a73_cluster_pwr_dwn
