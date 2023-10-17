/*
 * Copyright (c) 2020-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <neoverse_n2.h>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse N2 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-N2 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table NEOVERSE_N2_BHB_LOOP_COUNT, neoverse_n2
#endif /* WORKAROUND_CVE_2022_23960 */

/*
 * ERRATA_DSU_2313941:
 * The errata is defined in dsu_helpers.S and applies to Neoverse N2.
 * Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_neoverse_n2_2313941, check_errata_dsu_2313941
.equ erratum_neoverse_n2_2313941_wa, errata_dsu_2313941_wa
add_erratum_entry neoverse_n2, ERRATUM(2313941), ERRATA_DSU_2313941, APPLY_AT_RESET

workaround_reset_start neoverse_n2, ERRATUM(2002655), ERRATA_N2_2002655
	/* Apply instruction patching sequence */
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
workaround_reset_end neoverse_n2, ERRATUM(2002655)

check_erratum_ls neoverse_n2, ERRATUM(2002655), CPU_REV(0, 0)

workaround_reset_start neoverse_n2, ERRATUM(2025414), ERRATA_N2_2025414
	sysreg_bit_set NEOVERSE_N2_CPUECTLR_EL1, NEOVERSE_N2_CPUECTLR_EL1_PFSTIDIS_BIT
workaround_reset_end neoverse_n2, ERRATUM(2025414)

check_erratum_ls neoverse_n2, ERRATUM(2025414), CPU_REV(0, 0)

workaround_reset_start neoverse_n2, ERRATUM(2067956), ERRATA_N2_2067956
	sysreg_bit_set NEOVERSE_N2_CPUACTLR_EL1, NEOVERSE_N2_CPUACTLR_EL1_BIT_46
workaround_reset_end neoverse_n2, ERRATUM(2067956)

check_erratum_ls neoverse_n2, ERRATUM(2067956), CPU_REV(0, 0)

workaround_runtime_start neoverse_n2, ERRATUM(2009478), ERRATA_N2_2009478
	/* Stash ERRSELR_EL1 in x2 */
	mrs     x2, ERRSELR_EL1

	/* Select error record 0 and clear ED bit */
	msr     ERRSELR_EL1, xzr
	mrs     x1, ERXCTLR_EL1
	bfi     x1, xzr, #ERXCTLR_ED_SHIFT, #1
	msr     ERXCTLR_EL1, x1

	/* Restore ERRSELR_EL1 from x2 */
	msr     ERRSELR_EL1, x2
workaround_runtime_end neoverse_n2, ERRATUM(2009478), NO_ISB

check_erratum_ls neoverse_n2, ERRATUM(2009478), CPU_REV(0, 0)

workaround_reset_start neoverse_n2, ERRATUM(2138953), ERRATA_N2_2138953
	/* Apply instruction patching sequence */
	mrs	x1, NEOVERSE_N2_CPUECTLR2_EL1
	mov	x0, #NEOVERSE_N2_CPUECTLR2_EL1_PF_MODE_CNSRV
	bfi	x1, x0, #CPUECTLR2_EL1_PF_MODE_LSB, #CPUECTLR2_EL1_PF_MODE_WIDTH
	msr	NEOVERSE_N2_CPUECTLR2_EL1, x1
workaround_reset_end neoverse_n2, ERRATUM(2138953)

check_erratum_ls neoverse_n2, ERRATUM(2138953), CPU_REV(0, 3)

workaround_reset_start neoverse_n2, ERRATUM(2138956), ERRATA_N2_2138956
	/* Apply instruction patching sequence */
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
workaround_reset_end neoverse_n2, ERRATUM(2138956)

check_erratum_ls neoverse_n2, ERRATUM(2138956), CPU_REV(0, 0)


workaround_reset_start neoverse_n2, ERRATUM(2138958), ERRATA_N2_2138958
	/* Apply instruction patching sequence */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR5_EL1, NEOVERSE_N2_CPUACTLR5_EL1_BIT_13
workaround_reset_end neoverse_n2, ERRATUM(2138958)

check_erratum_ls neoverse_n2, ERRATUM(2138958), CPU_REV(0, 0)

workaround_reset_start neoverse_n2, ERRATUM(2189731), ERRATA_N2_2189731
	sysreg_bit_set NEOVERSE_N2_CPUACTLR5_EL1, NEOVERSE_N2_CPUACTLR5_EL1_BIT_44
workaround_reset_end neoverse_n2, ERRATUM(2189731)

check_erratum_ls neoverse_n2, ERRATUM(2189731), CPU_REV(0, 0)

workaround_reset_start neoverse_n2, ERRATUM(2242400), ERRATA_N2_2242400
	/* Apply instruction patching sequence */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR5_EL1, NEOVERSE_N2_CPUACTLR5_EL1_BIT_17
	ldr	x0, =0x2
	msr	S3_6_c15_c8_0, x0
	ldr	x0, =0x10F600E000
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FF80E000
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x80000000003FF
	msr	S3_6_c15_c8_1, x0
workaround_reset_end neoverse_n2, ERRATUM(2242400)

check_erratum_ls neoverse_n2, ERRATUM(2242400), CPU_REV(0, 0)

workaround_reset_start neoverse_n2, ERRATUM(2242415), ERRATA_N2_2242415
	sysreg_bit_set NEOVERSE_N2_CPUACTLR_EL1, NEOVERSE_N2_CPUACTLR_EL1_BIT_22
workaround_reset_end neoverse_n2, ERRATUM(2242415)

check_erratum_ls neoverse_n2, ERRATUM(2242415), CPU_REV(0, 0)

workaround_reset_start neoverse_n2, ERRATUM(2280757), ERRATA_N2_2280757
	/* Apply instruction patching sequence */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR_EL1, NEOVERSE_N2_CPUACTLR_EL1_BIT_22
workaround_reset_end neoverse_n2, ERRATUM(2280757)

check_erratum_ls neoverse_n2, ERRATUM(2280757), CPU_REV(0, 0)

workaround_runtime_start neoverse_n2, ERRATUM(2326639), ERRATA_N2_2326639
	/* Set bit 36 in ACTLR2_EL1 */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR2_EL1, NEOVERSE_N2_CPUACTLR2_EL1_BIT_36
workaround_runtime_end neoverse_n2, ERRATUM(2326639)

check_erratum_ls neoverse_n2, ERRATUM(2326639), CPU_REV(0, 0)

workaround_runtime_start neoverse_n2, ERRATUM(2340933), ERRATA_N2_2340933
	/* Set bit 61 in CPUACTLR5_EL1 */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR5_EL1, BIT(61)
workaround_runtime_end neoverse_n2, ERRATUM(2340933)

check_erratum_ls neoverse_n2, ERRATUM(2340933), CPU_REV(0, 0)

workaround_runtime_start neoverse_n2, ERRATUM(2346952), ERRATA_N2_2346952
	/* Set TXREQ to STATIC and full L2 TQ size */
	mrs	x1, NEOVERSE_N2_CPUECTLR2_EL1
	mov	x0, #CPUECTLR2_EL1_TXREQ_STATIC_FULL
	bfi	x1, x0, #CPUECTLR2_EL1_TXREQ_LSB, #CPUECTLR2_EL1_TXREQ_WIDTH
	msr	NEOVERSE_N2_CPUECTLR2_EL1, x1
workaround_runtime_end neoverse_n2, ERRATUM(2346952)

check_erratum_ls neoverse_n2, ERRATUM(2346952), CPU_REV(0, 2)

workaround_reset_start neoverse_n2, ERRATUM(2376738), ERRATA_N2_2376738
	/* Set CPUACTLR2_EL1[0] to 1 to force PLDW/PFRM
	 * ST to behave like PLD/PFRM LD and not cause
	 * invalidations to other PE caches.
	 */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR2_EL1, NEOVERSE_N2_CPUACTLR2_EL1_BIT_0
workaround_reset_end neoverse_n2, ERRATUM(2376738)

check_erratum_ls neoverse_n2, ERRATUM(2376738), CPU_REV(0, 3)

workaround_reset_start neoverse_n2, ERRATUM(2388450), ERRATA_N2_2388450
	/*Set bit 40 in ACTLR2_EL1 */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR2_EL1, NEOVERSE_N2_CPUACTLR2_EL1_BIT_40
workaround_reset_end neoverse_n2, ERRATUM(2388450)

check_erratum_ls neoverse_n2, ERRATUM(2388450), CPU_REV(0, 0)

workaround_reset_start neoverse_n2, ERRATUM(2743014), ERRATA_N2_2743014
	/* Set CPUACTLR5_EL1[56:55] to 2'b01 */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR5_EL1, NEOVERSE_N2_CPUACTLR5_EL1_BIT_55
	sysreg_bit_clear NEOVERSE_N2_CPUACTLR5_EL1, NEOVERSE_N2_CPUACTLR5_EL1_BIT_56
workaround_reset_end neoverse_n2, ERRATUM(2743014)

check_erratum_ls neoverse_n2, ERRATUM(2743014), CPU_REV(0, 2)

workaround_runtime_start neoverse_n2, ERRATUM(2743089), ERRATA_N2_2743089
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end neoverse_n2, ERRATUM(2743089)

check_erratum_ls neoverse_n2, ERRATUM(2743089), CPU_REV(0, 2)

workaround_reset_start neoverse_n2, ERRATUM(2779511), ERRATA_N2_2779511
	/* Set bit 47 in ACTLR3_EL1 */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR3_EL1, NEOVERSE_N2_CPUACTLR3_EL1_BIT_47
workaround_reset_end neoverse_n2, ERRATUM(2779511)

check_erratum_ls neoverse_n2, ERRATUM(2779511), CPU_REV(0, 2)

workaround_reset_start neoverse_n2, CVE(2022,23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Neoverse-N2 generic vectors are overridden to apply errata
         * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_neoverse_n2
#endif /* IMAGE_BL31 */
workaround_reset_end neoverse_n2, CVE(2022,23960)

check_erratum_chosen neoverse_n2, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

	/* -------------------------------------------
	 * The CPU Ops reset function for Neoverse N2.
	 * -------------------------------------------
	 */
cpu_reset_func_start neoverse_n2

	/* Check if the PE implements SSBS */
	mrs	x0, id_aa64pfr1_el1
	tst	x0, #(ID_AA64PFR1_EL1_SSBS_MASK << ID_AA64PFR1_EL1_SSBS_SHIFT)
	b.eq	1f

	/* Disable speculative loads */
	msr	SSBS, xzr
1:
	/* Force all cacheable atomic instructions to be near */
	sysreg_bit_set NEOVERSE_N2_CPUACTLR2_EL1, NEOVERSE_N2_CPUACTLR2_EL1_BIT_2

#if ENABLE_FEAT_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	sysreg_bit_set cptr_el3, TAM_BIT
	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	sysreg_bit_set cptr_el2, TAM_BIT
	/* No need to enable the counters as this would be done at el3 exit */
#endif

#if NEOVERSE_Nx_EXTERNAL_LLC
	/* Some systems may have External LLC, core needs to be made aware */
	sysreg_bit_set NEOVERSE_N2_CPUECTLR_EL1, NEOVERSE_N2_CPUECTLR_EL1_EXTLLC_BIT
#endif
cpu_reset_func_end neoverse_n2

func neoverse_n2_core_pwr_dwn

	apply_erratum neoverse_n2, ERRATUM(2009478), ERRATA_N2_2009478
	apply_erratum neoverse_n2, ERRATUM(2326639), ERRATA_N2_2326639, NO_GET_CPU_REV

	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * No need to do cache maintenance here.
	 * ---------------------------------------------------
	 */
	sysreg_bit_set NEOVERSE_N2_CPUPWRCTLR_EL1, NEOVERSE_N2_CORE_PWRDN_EN_BIT

	apply_erratum neoverse_n2, ERRATUM(2743089), ERRATA_N2_2743089

	isb
	ret
endfunc neoverse_n2_core_pwr_dwn

errata_report_shim neoverse_n2

	/* ---------------------------------------------
	 * This function provides Neoverse N2 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ASCII and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_n2_regs, "aS"
neoverse_n2_regs:  /* The ASCII list of register names to be reported */
	.asciz	"cpupwrctlr_el1", ""

func neoverse_n2_cpu_reg_dump
	adr	x6, neoverse_n2_regs
	mrs	x8, NEOVERSE_N2_CPUPWRCTLR_EL1
	ret
endfunc neoverse_n2_cpu_reg_dump

declare_cpu_ops neoverse_n2, NEOVERSE_N2_MIDR, \
	neoverse_n2_reset_func, \
	neoverse_n2_core_pwr_dwn
