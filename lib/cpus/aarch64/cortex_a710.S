/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a710.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex A710 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex A710 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A710_BHB_LOOP_COUNT, cortex_a710
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_a710, ERRATUM(1987031), ERRATA_A710_1987031
	ldr x0,=0x6
	msr S3_6_c15_c8_0,x0
	ldr x0,=0xF3A08002
	msr S3_6_c15_c8_2,x0
	ldr x0,=0xFFF0F7FE
	msr S3_6_c15_c8_3,x0
	ldr x0,=0x40000001003ff
	msr S3_6_c15_c8_1,x0
	ldr x0,=0x7
	msr S3_6_c15_c8_0,x0
	ldr x0,=0xBF200000
	msr S3_6_c15_c8_2,x0
	ldr x0,=0xFFEF0000
	msr S3_6_c15_c8_3,x0
	ldr x0,=0x40000001003f3
	msr S3_6_c15_c8_1,x0
workaround_reset_end cortex_a710, ERRATUM(1987031)

check_erratum_ls cortex_a710, ERRATUM(1987031), CPU_REV(2, 0)

workaround_runtime_start cortex_a710, ERRATUM(2008768), ERRATA_A710_2008768
	/* Stash ERRSELR_EL1 in x2 */
	mrs	x2, ERRSELR_EL1

	/* Select error record 0 and clear ED bit */
	msr	ERRSELR_EL1, xzr
	mrs	x1, ERXCTLR_EL1
	bfi	x1, xzr, #ERXCTLR_ED_SHIFT, #1
	msr	ERXCTLR_EL1, x1

	/* Select error record 1 and clear ED bit */
	mov	x0, #1
	msr	ERRSELR_EL1, x0
	mrs	x1, ERXCTLR_EL1
	bfi	x1, xzr, #ERXCTLR_ED_SHIFT, #1
	msr	ERXCTLR_EL1, x1

	/* Restore ERRSELR_EL1 from x2 */
	msr	ERRSELR_EL1, x2
workaround_runtime_end cortex_a710, ERRATUM(2008768), NO_ISB

check_erratum_ls cortex_a710, ERRATUM(2008768), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2017096), ERRATA_A710_2017096
	sysreg_bit_set	CORTEX_A710_CPUECTLR_EL1, CORTEX_A710_CPUECTLR_EL1_PFSTIDIS_BIT
workaround_reset_end cortex_a710, ERRATUM(2017096)

check_erratum_ls cortex_a710, ERRATUM(2017096), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2055002), ERRATA_A710_2055002
	sysreg_bit_set	CORTEX_A710_CPUACTLR_EL1, CORTEX_A710_CPUACTLR_EL1_BIT_46
workaround_reset_end cortex_a710, ERRATUM(2055002)

check_erratum_range cortex_a710, ERRATUM(2055002), CPU_REV(1, 0), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2058056), ERRATA_A710_2058056
	sysreg_bitfield_insert CORTEX_A710_CPUECTLR2_EL1, CORTEX_A710_CPUECTLR2_EL1_PF_MODE_CNSRV, \
		CPUECTLR2_EL1_PF_MODE_LSB, CPUECTLR2_EL1_PF_MODE_WIDTH
workaround_reset_end cortex_a710, ERRATUM(2058056)

check_erratum_ls cortex_a710, ERRATUM(2058056), CPU_REV(2, 1)

workaround_reset_start cortex_a710, ERRATUM(2081180), ERRATA_A710_2081180
	ldr	x0,=0x3
	msr	S3_6_c15_c8_0,x0
	ldr	x0,=0xF3A08002
	msr	S3_6_c15_c8_2,x0
	ldr	x0,=0xFFF0F7FE
	msr	S3_6_c15_c8_3,x0
	ldr	x0,=0x10002001003FF
	msr	S3_6_c15_c8_1,x0
	ldr	x0,=0x4
	msr	S3_6_c15_c8_0,x0
	ldr	x0,=0xBF200000
	msr	S3_6_c15_c8_2,x0
	ldr	x0,=0xFFEF0000
	msr	S3_6_c15_c8_3,x0
	ldr	x0,=0x10002001003F3
	msr	S3_6_c15_c8_1,x0
workaround_reset_end cortex_a710, ERRATUM(2081180)

check_erratum_ls cortex_a710, ERRATUM(2081180), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2083908), ERRATA_A710_2083908
	sysreg_bit_set	CORTEX_A710_CPUACTLR5_EL1, CORTEX_A710_CPUACTLR5_EL1_BIT_13
workaround_reset_end cortex_a710, ERRATUM(2083908)

check_erratum_range cortex_a710, ERRATUM(2083908), CPU_REV(2, 0), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2136059), ERRATA_A710_2136059
	sysreg_bit_set	CORTEX_A710_CPUACTLR5_EL1, CORTEX_A710_CPUACTLR5_EL1_BIT_44
workaround_reset_end cortex_a710, ERRATUM(2136059)

check_erratum_ls cortex_a710, ERRATUM(2136059), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2147715), ERRATA_A710_2147715
	sysreg_bit_set	CORTEX_A710_CPUACTLR_EL1, CORTEX_A710_CPUACTLR_EL1_BIT_22
workaround_reset_end cortex_a710, ERRATUM(2147715)

check_erratum_range cortex_a710, ERRATUM(2147715), CPU_REV(2, 0), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2216384), ERRATA_A710_2216384
	sysreg_bit_set	CORTEX_A710_CPUACTLR5_EL1, CORTEX_A710_CPUACTLR5_EL1_BIT_17

	ldr	x0,=0x5
	msr	CORTEX_A710_CPUPSELR_EL3, x0
	ldr	x0,=0x10F600E000
	msr	CORTEX_A710_CPUPOR_EL3, x0
	ldr	x0,=0x10FF80E000
	msr	CORTEX_A710_CPUPMR_EL3, x0
	ldr	x0,=0x80000000003FF
	msr	CORTEX_A710_CPUPCR_EL3, x0
workaround_reset_end cortex_a710, ERRATUM(2216384)

check_erratum_ls cortex_a710, ERRATUM(2216384), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2267065), ERRATA_A710_2267065
	sysreg_bit_set	CORTEX_A710_CPUACTLR_EL1, CORTEX_A710_CPUACTLR_EL1_BIT_22
workaround_reset_end cortex_a710, ERRATUM(2267065)

check_erratum_ls cortex_a710, ERRATUM(2267065), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2282622), ERRATA_A710_2282622
	sysreg_bit_set	CORTEX_A710_CPUACTLR2_EL1, BIT(0)
workaround_reset_end cortex_a710, ERRATUM(2282622)

check_erratum_ls cortex_a710, ERRATUM(2282622), CPU_REV(2, 1)

workaround_runtime_start cortex_a710, ERRATUM(2291219), ERRATA_A710_2291219
	/* Set bit 36 in ACTLR2_EL1 */
	sysreg_bit_set CORTEX_A710_CPUACTLR2_EL1, CORTEX_A710_CPUACTLR2_EL1_BIT_36
workaround_runtime_end cortex_a710, ERRATUM(2291219), NO_ISB

check_erratum_ls cortex_a710, ERRATUM(2291219), CPU_REV(2, 0)

/*
 * ERRATA_DSU_2313941 is defined in dsu_helpers.S but applies to Cortex-A710 as
 * well. Create a symbollic link to existing errata workaround to get them
 * registered under the Errata Framework.
 */
.equ check_erratum_cortex_a710_2313941, check_errata_dsu_2313941
.equ erratum_cortex_a710_2313941_wa, errata_dsu_2313941_wa
add_erratum_entry cortex_a710, ERRATUM(2313941), ERRATA_DSU_2313941, APPLY_AT_RESET

workaround_reset_start cortex_a710, ERRATUM(2371105), ERRATA_A710_2371105
	/* Set bit 40 in CPUACTLR2_EL1 */
	sysreg_bit_set CORTEX_A710_CPUACTLR2_EL1, CORTEX_A710_CPUACTLR2_EL1_BIT_40
workaround_reset_end cortex_a710, ERRATUM(2371105)

check_erratum_ls cortex_a710, ERRATUM(2371105), CPU_REV(2, 0)

workaround_reset_start cortex_a710, ERRATUM(2742423), ERRATA_A710_2742423
	/* Set CPUACTLR5_EL1[56:55] to 2'b01 */
	sysreg_bit_set CORTEX_A710_CPUACTLR5_EL1, BIT(55)
	sysreg_bit_clear CORTEX_A710_CPUACTLR5_EL1, BIT(56)
workaround_reset_end cortex_a710, ERRATUM(2742423)

check_erratum_ls cortex_a710, ERRATUM(2742423), CPU_REV(2, 1)

workaround_runtime_start cortex_a710, ERRATUM(2768515), ERRATA_A710_2768515
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end cortex_a710, ERRATUM(2768515), NO_ISB

check_erratum_ls cortex_a710, ERRATUM(2768515), CPU_REV(2, 1)

workaround_reset_start cortex_a710, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-A710 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_a710
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a710, CVE(2022, 23960)

check_erratum_chosen cortex_a710, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a710_core_pwr_dwn
	apply_erratum cortex_a710, ERRATUM(2008768), ERRATA_A710_2008768
	apply_erratum cortex_a710, ERRATUM(2291219), ERRATA_A710_2291219, NO_GET_CPU_REV

	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A710_CPUPWRCTLR_EL1, CORTEX_A710_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	apply_erratum cortex_a710, ERRATUM(2768515), ERRATA_A710_2768515, NO_GET_CPU_REV
	isb
	ret
endfunc cortex_a710_core_pwr_dwn

errata_report_shim cortex_a710

cpu_reset_func_start cortex_a710
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_a710

	/* ---------------------------------------------
	 * This function provides Cortex-A710 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a710_regs, "aS"
cortex_a710_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a710_cpu_reg_dump
	adr	x6, cortex_a710_regs
	mrs	x8, CORTEX_A710_CPUECTLR_EL1
	ret
endfunc cortex_a710_cpu_reg_dump

declare_cpu_ops cortex_a710, CORTEX_A710_MIDR, \
	cortex_a710_reset_func, \
	cortex_a710_core_pwr_dwn
