/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_x2.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex X2 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex X2 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_X2_BHB_LOOP_COUNT, cortex_x2
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_x2, ERRATUM(2002765), ERRATA_X2_2002765
	ldr	x0, =0x6
	msr	S3_6_C15_C8_0, x0 /* CPUPSELR_EL3 */
	ldr	x0, =0xF3A08002
	msr	S3_6_C15_C8_2, x0 /* CPUPOR_EL3 */
	ldr	x0, =0xFFF0F7FE
	msr	S3_6_C15_C8_3, x0 /* CPUPMR_EL3 */
	ldr	x0, =0x40000001003ff
	msr	S3_6_C15_C8_1, x0 /* CPUPCR_EL3 */
workaround_reset_end cortex_x2, ERRATUM(2002765)

check_erratum_ls cortex_x2, ERRATUM(2002765), CPU_REV(2, 0)

workaround_reset_start cortex_x2, ERRATUM(2017096), ERRATA_X2_2017096
	sysreg_bit_set CORTEX_X2_CPUECTLR_EL1, CORTEX_X2_CPUECTLR_EL1_PFSTIDIS_BIT
workaround_reset_end cortex_x2, ERRATUM(2017096)

check_erratum_ls cortex_x2, ERRATUM(2017096), CPU_REV(2, 0)

workaround_reset_start cortex_x2, ERRATUM(2058056), ERRATA_X2_2058056
	sysreg_bitfield_insert CORTEX_X2_CPUECTLR2_EL1, CORTEX_X2_CPUECTLR2_EL1_PF_MODE_CNSRV, \
	CORTEX_X2_CPUECTLR2_EL1_PF_MODE_SHIFT, CORTEX_X2_CPUECTLR2_EL1_PF_MODE_WIDTH
workaround_reset_end cortex_x2, ERRATUM(2058056)

check_erratum_ls cortex_x2, ERRATUM(2058056), CPU_REV(2, 1)

workaround_reset_start cortex_x2, ERRATUM(2081180), ERRATA_X2_2081180
	/* Apply instruction patching sequence */
	ldr	x0, =0x3
	msr	CORTEX_X2_IMP_CPUPSELR_EL3, x0
	ldr	x0, =0xF3A08002
	msr	CORTEX_X2_IMP_CPUPOR_EL3, x0
	ldr	x0, =0xFFF0F7FE
	msr	CORTEX_X2_IMP_CPUPMR_EL3, x0
	ldr	x0, =0x10002001003FF
	msr	CORTEX_X2_IMP_CPUPCR_EL3, x0
	ldr	x0, =0x4
	msr	CORTEX_X2_IMP_CPUPSELR_EL3, x0
	ldr	x0, =0xBF200000
	msr	CORTEX_X2_IMP_CPUPOR_EL3, x0
	ldr	x0, =0xFFEF0000
	msr	CORTEX_X2_IMP_CPUPMR_EL3, x0
	ldr	x0, =0x10002001003F3
	msr	CORTEX_X2_IMP_CPUPCR_EL3, x0
workaround_reset_end cortex_x2, ERRATUM(2081180)

check_erratum_ls cortex_x2, ERRATUM(2081180), CPU_REV(2, 0)

workaround_reset_start cortex_x2, ERRATUM(2083908), ERRATA_X2_2083908
	/* Apply the workaround by setting bit 13 in CPUACTLR5_EL1. */
	sysreg_bit_set CORTEX_X2_CPUACTLR5_EL1, BIT(13)
workaround_reset_end cortex_x2, ERRATUM(2083908)

check_erratum_range cortex_x2, ERRATUM(2083908), CPU_REV(2, 0), CPU_REV(2, 0)

workaround_reset_start cortex_x2, ERRATUM(2147715), ERRATA_X2_2147715
	/* Apply the workaround by setting bit 22 in CPUACTLR_EL1. */
	sysreg_bit_set CORTEX_X2_CPUACTLR_EL1, CORTEX_X2_CPUACTLR_EL1_BIT_22
workaround_reset_end cortex_x2, ERRATUM(2147715)

check_erratum_range cortex_x2, ERRATUM(2147715), CPU_REV(2, 0), CPU_REV(2, 0)

workaround_reset_start cortex_x2, ERRATUM(2216384), ERRATA_X2_2216384
	sysreg_bit_set CORTEX_X2_CPUACTLR5_EL1, CORTEX_X2_CPUACTLR5_EL1_BIT_17

	/* Apply instruction patching sequence */
	ldr	x0, =0x5
	msr	CORTEX_X2_IMP_CPUPSELR_EL3, x0
	ldr	x0, =0x10F600E000
	msr	CORTEX_X2_IMP_CPUPOR_EL3, x0
	ldr	x0, =0x10FF80E000
	msr	CORTEX_X2_IMP_CPUPMR_EL3, x0
	ldr	x0, =0x80000000003FF
	msr	CORTEX_X2_IMP_CPUPCR_EL3, x0
workaround_reset_end cortex_x2, ERRATUM(2216384)

check_erratum_ls cortex_x2, ERRATUM(2216384), CPU_REV(2, 0)

workaround_reset_start cortex_x2, ERRATUM(2282622), ERRATA_X2_2282622
	/* Apply the workaround */
	sysreg_bit_set CORTEX_X2_CPUACTLR2_EL1, BIT(0)
workaround_reset_end cortex_x2, ERRATUM(2282622)

check_erratum_ls cortex_x2, ERRATUM(2282622), CPU_REV(2, 1)

workaround_reset_start cortex_x2, ERRATUM(2371105), ERRATA_X2_2371105
	/* Set bit 40 in CPUACTLR2_EL1 */
	sysreg_bit_set CORTEX_X2_CPUACTLR2_EL1, CORTEX_X2_CPUACTLR2_EL1_BIT_40
workaround_reset_end cortex_x2, ERRATUM(2371105)

check_erratum_ls cortex_x2, ERRATUM(2371105), CPU_REV(2, 0)

workaround_reset_start cortex_x2, ERRATUM(2742423), ERRATA_X2_2742423
	/* Set CPUACTLR5_EL1[56:55] to 2'b01 */
	sysreg_bit_set CORTEX_X2_CPUACTLR5_EL1, BIT(55)
	sysreg_bit_clear CORTEX_X2_CPUACTLR5_EL1, BIT(56)
workaround_reset_end cortex_x2, ERRATUM(2742423)

check_erratum_ls cortex_x2, ERRATUM(2742423), CPU_REV(2, 1)

workaround_reset_start cortex_x2, ERRATUM(2768515), ERRATA_X2_2768515
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_reset_end cortex_x2, ERRATUM(2768515)

check_erratum_ls cortex_x2, ERRATUM(2768515), CPU_REV(2, 1)

workaround_reset_start cortex_x2, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-X2 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_x2
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_x2, CVE(2022, 23960)

check_erratum_chosen cortex_x2, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

/*
 * ERRATA_DSU_2313941 :
 * The errata is defined in dsu_helpers.S but applies to cortex_x2
 * as well. Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_cortex_x2_2313941, check_errata_dsu_2313941
.equ erratum_cortex_x2_2313941_wa, errata_dsu_2313941_wa
add_erratum_entry cortex_x2, ERRATUM(2313941), ERRATA_DSU_2313941, APPLY_AT_RESET

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_x2_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_X2_CPUPWRCTLR_EL1, CORTEX_X2_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

#if ERRATA_X2_2768515
	mov	x15, x30
	bl	cpu_get_rev_var
	bl	erratum_cortex_x2_2768515_wa
	mov	x30, x15
#endif /* ERRATA_X2_2768515 */
	isb
	ret
endfunc cortex_x2_core_pwr_dwn

errata_report_shim cortex_x2

cpu_reset_func_start cortex_x2
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_x2

	/* ---------------------------------------------
	 * This function provides Cortex X2 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_x2_regs, "aS"
cortex_x2_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x2_cpu_reg_dump
	adr	x6, cortex_x2_regs
	mrs	x8, CORTEX_X2_CPUECTLR_EL1
	ret
endfunc cortex_x2_cpu_reg_dump

declare_cpu_ops cortex_x2, CORTEX_X2_MIDR, \
	cortex_x2_reset_func, \
	cortex_x2_core_pwr_dwn
