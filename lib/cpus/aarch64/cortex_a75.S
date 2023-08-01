/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cortex_a75.h>
#include <cpuamu.h>
#include <cpu_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A75 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

workaround_reset_start cortex_a75, ERRATUM(764081), ERRATA_A75_764081
	sysreg_bit_set sctlr_el3, SCTLR_IESB_BIT
workaround_reset_end cortex_a75, ERRATUM(764081)

check_erratum_ls cortex_a75, ERRATUM(764081), CPU_REV(0, 0)

workaround_reset_start cortex_a75, ERRATUM(790748), ERRATA_A75_790748
	sysreg_bit_set CORTEX_A75_CPUACTLR_EL1, (1 << 13)
workaround_reset_end cortex_a75, ERRATUM(790748)

check_erratum_ls cortex_a75, ERRATUM(790748), CPU_REV(0, 0)

/* ERRATA_DSU_798953 :
 * The errata is defined in dsu_helpers.S but applies to cortex_a75
 * as well. Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_cortex_a75_798953, check_errata_dsu_798953
.equ erratum_cortex_a75_798953_wa, errata_dsu_798953_wa
add_erratum_entry cortex_a75, ERRATUM(798953), ERRATA_DSU_798953, APPLY_AT_RESET

/* ERRATA_DSU_936184 :
 * The errata is defined in dsu_helpers.S but applies to cortex_a75
 * as well. Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_cortex_a75_936184, check_errata_dsu_936184
.equ erratum_cortex_a75_936184_wa, errata_dsu_936184_wa
add_erratum_entry cortex_a75, ERRATUM(936184), ERRATA_DSU_936184, APPLY_AT_RESET

workaround_reset_start cortex_a75, CVE(2017, 5715), WORKAROUND_CVE_2017_5715
#if IMAGE_BL31
	override_vector_table wa_cve_2017_5715_bpiall_vbar
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a75, CVE(2017, 5715)

check_erratum_custom_start cortex_a75, CVE(2017, 5715)
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
check_erratum_custom_end cortex_a75, CVE(2017, 5715)

workaround_reset_start cortex_a75, CVE(2018, 3639), WORKAROUND_CVE_2018_3639
	sysreg_bit_set CORTEX_A75_CPUACTLR_EL1, CORTEX_A75_CPUACTLR_EL1_DISABLE_LOAD_PASS_STORE
workaround_reset_end cortex_a75, CVE(2018, 3639)

check_erratum_chosen cortex_a75, CVE(2018, 3639), WORKAROUND_CVE_2018_3639

workaround_reset_start cortex_a75, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/* Skip installing vector table again if already done for CVE(2017, 5715) */
	adr	x0, wa_cve_2017_5715_bpiall_vbar
	mrs	x1, vbar_el3
	cmp	x0, x1
	b.eq	1f
	msr	vbar_el3, x0
1:
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a75, CVE(2022, 23960)

check_erratum_custom_start cortex_a75, CVE(2022, 23960)
#if WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960
	cpu_check_csv2	x0, 1f
	mov	x0, #ERRATA_APPLIES
	ret
1:
# if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
# else
	mov	x0, #ERRATA_MISSING
# endif /* WORKAROUND_CVE_2022_23960 */
	ret
#endif /* WORKAROUND_CVE_2017_5715 || WORKAROUND_CVE_2022_23960 */
	mov	x0, #ERRATA_MISSING
	ret
check_erratum_custom_end cortex_a75, CVE(2022, 23960)

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A75.
	 * -------------------------------------------------
	 */

cpu_reset_func_start cortex_a75
#if ENABLE_FEAT_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	sysreg_bit_set actlr_el3, CORTEX_A75_ACTLR_AMEN_BIT
	isb

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	sysreg_bit_set actlr_el2, CORTEX_A75_ACTLR_AMEN_BIT
	isb

	/* Enable group0 counters */
	mov	x0, #CORTEX_A75_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
	isb

	/* Enable group1 counters */
	mov	x0, #CORTEX_A75_AMU_GROUP1_MASK
	msr	CPUAMCNTENSET_EL0, x0
	/* isb included in cpu_reset_func_end macro */
#endif
cpu_reset_func_end cortex_a75

func check_smccc_arch_workaround_3
	mov	x0, #ERRATA_APPLIES
	ret
endfunc check_smccc_arch_workaround_3

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a75_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set CORTEX_A75_CPUPWRCTLR_EL1, \
		CORTEX_A75_CORE_PWRDN_EN_MASK
	isb
	ret
endfunc cortex_a75_core_pwr_dwn

errata_report_shim cortex_a75

	/* ---------------------------------------------
	 * This function provides cortex_a75 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a75_regs, "aS"
cortex_a75_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a75_cpu_reg_dump
	adr	x6, cortex_a75_regs
	mrs	x8, CORTEX_A75_CPUECTLR_EL1
	ret
endfunc cortex_a75_cpu_reg_dump

declare_cpu_ops_wa cortex_a75, CORTEX_A75_MIDR, \
	cortex_a75_reset_func, \
	check_erratum_cortex_a75_5715, \
	CPU_NO_EXTRA2_FUNC, \
	check_smccc_arch_workaround_3, \
	cortex_a75_core_pwr_dwn
