/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpuamu.h>
#include <cpu_macros.S>
#include <neoverse_n1.h>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse N1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-N1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	.global neoverse_n1_errata_ic_trap_handler

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table NEOVERSE_N1_BHB_LOOP_COUNT, neoverse_n1
#endif /* WORKAROUND_CVE_2022_23960 */

/*
 * ERRATA_DSU_936184:
 * The errata is defined in dsu_helpers.S and applies to Neoverse N1.
 * Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_neoverse_n1_936184, check_errata_dsu_936184
.equ erratum_neoverse_n1_936184_wa, errata_dsu_936184_wa
add_erratum_entry neoverse_n1, ERRATUM(936184), ERRATA_DSU_936184, APPLY_AT_RESET

workaround_reset_start neoverse_n1, ERRATUM(1043202), ERRATA_N1_1043202
	/* Apply instruction patching sequence */
	ldr	x0, =0x0
	msr	CPUPSELR_EL3, x0
	ldr	x0, =0xF3BF8F2F
	msr	CPUPOR_EL3, x0
	ldr	x0, =0xFFFFFFFF
	msr	CPUPMR_EL3, x0
	ldr	x0, =0x800200071
	msr	CPUPCR_EL3, x0
workaround_reset_end neoverse_n1, ERRATUM(1043202)

check_erratum_ls neoverse_n1, ERRATUM(1043202), CPU_REV(1, 0)

workaround_reset_start neoverse_n1, ERRATUM(1073348), ERRATA_N1_1073348
	sysreg_bit_set NEOVERSE_N1_CPUACTLR_EL1, NEOVERSE_N1_CPUACTLR_EL1_BIT_6
workaround_reset_end neoverse_n1, ERRATUM(1073348)

check_erratum_ls neoverse_n1, ERRATUM(1073348), CPU_REV(1, 0)

workaround_reset_start neoverse_n1, ERRATUM(1130799), ERRATA_N1_1130799
	sysreg_bit_set NEOVERSE_N1_CPUACTLR2_EL1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_59
workaround_reset_end neoverse_n1, ERRATUM(1130799)

check_erratum_ls neoverse_n1, ERRATUM(1130799), CPU_REV(2, 0)

workaround_reset_start neoverse_n1, ERRATUM(1165347), ERRATA_N1_1165347
	sysreg_bit_set NEOVERSE_N1_CPUACTLR2_EL1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_0
	sysreg_bit_set NEOVERSE_N1_CPUACTLR2_EL1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_15
workaround_reset_end neoverse_n1, ERRATUM(1165347)

check_erratum_ls neoverse_n1, ERRATUM(1165347), CPU_REV(2, 0)

workaround_reset_start neoverse_n1, ERRATUM(1207823), ERRATA_N1_1207823
	sysreg_bit_set NEOVERSE_N1_CPUACTLR2_EL1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_11
workaround_reset_end neoverse_n1, ERRATUM(1207823)

check_erratum_ls neoverse_n1, ERRATUM(1207823), CPU_REV(2, 0)

workaround_reset_start neoverse_n1, ERRATUM(1220197), ERRATA_N1_1220197
	sysreg_bit_set NEOVERSE_N1_CPUECTLR_EL1, NEOVERSE_N1_WS_THR_L2_MASK
workaround_reset_end neoverse_n1, ERRATUM(1220197)

check_erratum_ls neoverse_n1, ERRATUM(1220197), CPU_REV(2, 0)

workaround_reset_start neoverse_n1, ERRATUM(1257314), ERRATA_N1_1257314
	sysreg_bit_set NEOVERSE_N1_CPUACTLR3_EL1, NEOVERSE_N1_CPUACTLR3_EL1_BIT_10
workaround_reset_end neoverse_n1, ERRATUM(1257314)

check_erratum_ls neoverse_n1, ERRATUM(1257314), CPU_REV(3, 0)

workaround_reset_start neoverse_n1, ERRATUM(1262606), ERRATA_N1_1262606
	sysreg_bit_set NEOVERSE_N1_CPUACTLR_EL1, NEOVERSE_N1_CPUACTLR_EL1_BIT_13
workaround_reset_end neoverse_n1, ERRATUM(1262606)

check_erratum_ls neoverse_n1, ERRATUM(1262606), CPU_REV(3, 0)

workaround_reset_start neoverse_n1, ERRATUM(1262888), ERRATA_N1_1262888
	sysreg_bit_set NEOVERSE_N1_CPUECTLR_EL1, NEOVERSE_N1_CPUECTLR_EL1_MM_TLBPF_DIS_BIT
workaround_reset_end neoverse_n1, ERRATUM(1262888)

check_erratum_ls neoverse_n1, ERRATUM(1262888), CPU_REV(3, 0)

workaround_reset_start neoverse_n1, ERRATUM(1275112), ERRATA_N1_1275112
	sysreg_bit_set NEOVERSE_N1_CPUACTLR_EL1, NEOVERSE_N1_CPUACTLR_EL1_BIT_13
workaround_reset_end neoverse_n1, ERRATUM(1275112)

check_erratum_ls neoverse_n1, ERRATUM(1275112), CPU_REV(3, 0)

workaround_reset_start neoverse_n1, ERRATUM(1315703), ERRATA_N1_1315703
	sysreg_bit_set NEOVERSE_N1_CPUACTLR2_EL1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_16
workaround_reset_end neoverse_n1, ERRATUM(1315703)

check_erratum_ls neoverse_n1, ERRATUM(1315703), CPU_REV(3, 0)

workaround_reset_start neoverse_n1, ERRATUM(1542419), ERRATA_N1_1542419
	/* Apply instruction patching sequence */
	ldr	x0, =0x0
	msr	CPUPSELR_EL3, x0
	ldr	x0, =0xEE670D35
	msr	CPUPOR_EL3, x0
	ldr	x0, =0xFFFF0FFF
	msr	CPUPMR_EL3, x0
	ldr	x0, =0x08000020007D
	msr	CPUPCR_EL3, x0
	isb
workaround_reset_end neoverse_n1, ERRATUM(1542419)

check_erratum_range neoverse_n1, ERRATUM(1542419), CPU_REV(3, 0), CPU_REV(4, 0)

workaround_reset_start neoverse_n1, ERRATUM(1868343), ERRATA_N1_1868343
	sysreg_bit_set NEOVERSE_N1_CPUACTLR_EL1, NEOVERSE_N1_CPUACTLR_EL1_BIT_13
workaround_reset_end neoverse_n1, ERRATUM(1868343)

check_erratum_ls neoverse_n1, ERRATUM(1868343), CPU_REV(4, 0)

workaround_reset_start neoverse_n1, ERRATUM(1946160), ERRATA_N1_1946160
	mov	x0, #3
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3900002
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0
	mov	x0, #4
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3800082
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0
	mov	x0, #5
	msr	S3_6_C15_C8_0, x0
	ldr	x0, =0x10E3800200
	msr	S3_6_C15_C8_2, x0
	ldr	x0, =0x10FFF003E0
	msr	S3_6_C15_C8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_C15_C8_1, x0
	isb
workaround_reset_end neoverse_n1, ERRATUM(1946160)

check_erratum_range neoverse_n1, ERRATUM(1946160), CPU_REV(3, 0), CPU_REV(4, 1)

workaround_runtime_start neoverse_n1, ERRATUM(2743102), ERRATA_N1_2743102
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end neoverse_n1, ERRATUM(2743102)

check_erratum_ls neoverse_n1, ERRATUM(2743102), CPU_REV(4, 1)

workaround_reset_start neoverse_n1, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Neoverse-N1 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_neoverse_n1
#endif /* IMAGE_BL31 */
workaround_reset_end neoverse_n1, CVE(2022, 23960)

check_erratum_chosen neoverse_n1, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

/* --------------------------------------------------
 * Disable speculative loads if Neoverse N1 supports
 * SSBS.
 *
 * Shall clobber: x0.
 * --------------------------------------------------
 */
func neoverse_n1_disable_speculative_loads
	/* Check if the PE implements SSBS */
	mrs	x0, id_aa64pfr1_el1
	tst	x0, #(ID_AA64PFR1_EL1_SSBS_MASK << ID_AA64PFR1_EL1_SSBS_SHIFT)
	b.eq	1f

	/* Disable speculative loads */
	msr	SSBS, xzr

1:
	ret
endfunc neoverse_n1_disable_speculative_loads

cpu_reset_func_start neoverse_n1
	bl neoverse_n1_disable_speculative_loads

	/* Forces all cacheable atomic instructions to be near */
	sysreg_bit_set NEOVERSE_N1_CPUACTLR2_EL1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_2
	isb

#if ENABLE_FEAT_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	sysreg_bit_set actlr_el3, NEOVERSE_N1_ACTLR_AMEN_BIT
	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	sysreg_bit_set actlr_el2, NEOVERSE_N1_ACTLR_AMEN_BIT
	/* Enable group0 counters */
	mov	x0, #NEOVERSE_N1_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
#endif

#if NEOVERSE_Nx_EXTERNAL_LLC
	/* Some system may have External LLC, core needs to be made aware */
	sysreg_bit_set NEOVERSE_N1_CPUECTLR_EL1, NEOVERSE_N1_CPUECTLR_EL1_EXTLLC_BIT
#endif
cpu_reset_func_end neoverse_n1

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_n1_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set NEOVERSE_N1_CPUPWRCTLR_EL1, NEOVERSE_N1_CORE_PWRDN_EN_MASK

	apply_erratum neoverse_n1, ERRATUM(2743102), ERRATA_N1_2743102

	isb
	ret
endfunc neoverse_n1_core_pwr_dwn

errata_report_shim neoverse_n1

/*
 * Handle trap of EL0 IC IVAU instructions to EL3 by executing a TLB
 * inner-shareable invalidation to an arbitrary address followed by a DSB.
 *
 * x1: Exception Syndrome
 */
func neoverse_n1_errata_ic_trap_handler
	cmp	x1, #NEOVERSE_N1_EC_IC_TRAP
	b.ne	1f
	tlbi	vae3is, xzr
	dsb	sy

	# Skip the IC instruction itself
	mrs     x3, elr_el3
	add     x3, x3, #4
	msr     elr_el3, x3

	ldp	x0, x1, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X0]
	ldp	x2, x3, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X2]
	ldp	x4, x5, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_X4]
	ldr	x30, [sp, #CTX_GPREGS_OFFSET + CTX_GPREG_LR]

	/*
	 * Issue Error Synchronization Barrier to synchronize SErrors before
	 * exiting EL3. We're running with EAs unmasked, so any synchronized
	 * errors would be taken immediately; therefore no need to inspect
	 * DISR_EL1 register.
	 */
	esb
	exception_return
1:
	ret
endfunc neoverse_n1_errata_ic_trap_handler

	/* ---------------------------------------------
	 * This function provides neoverse_n1 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_n1_regs, "aS"
neoverse_n1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_n1_cpu_reg_dump
	adr	x6, neoverse_n1_regs
	mrs	x8, NEOVERSE_N1_CPUECTLR_EL1
	ret
endfunc neoverse_n1_cpu_reg_dump

declare_cpu_ops_eh neoverse_n1, NEOVERSE_N1_MIDR, \
	neoverse_n1_reset_func, \
	neoverse_n1_errata_ic_trap_handler, \
	neoverse_n1_core_pwr_dwn
