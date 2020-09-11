/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <neoverse_n1.h>
#include <cpuamu.h>
#include <cpu_macros.S>
#include <context.h>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse N1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-N1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	.global neoverse_n1_errata_ic_trap_handler
	.global is_scu_present_in_dsu

/*
 * Check DSU is configured with SCU and L3 unit
 * 1-> SCU present
 * 0-> SCU not present
 */
func is_scu_present_in_dsu
	mrs	x0, CPUCFR_EL1
	ubfx	x0, x0, #SCU_SHIFT, #1
	eor	x0, x0, #1
	ret
endfunc is_scu_present_in_dsu

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Erratum 1043202.
 * This applies to revision r0p0 and r1p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1043202_wa
	/* Compare x0 against revision r1p0 */
	mov	x17, x30
	bl	check_errata_1043202
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	ldr	x0, =0x0
	msr	CPUPSELR_EL3, x0
	ldr	x0, =0xF3BF8F2F
	msr	CPUPOR_EL3, x0
	ldr	x0, =0xFFFFFFFF
	msr	CPUPMR_EL3, x0
	ldr	x0, =0x800200071
	msr	CPUPCR_EL3, x0
	isb
1:
	ret	x17
endfunc errata_n1_1043202_wa

func check_errata_1043202
	/* Applies to r0p0 and r1p0 */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1043202

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

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1073348
 * This applies to revision r0p0 and r1p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1073348_wa
	/* Compare x0 against revision r1p0 */
	mov	x17, x30
	bl	check_errata_1073348
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUACTLR_EL1
	orr	x1, x1, NEOVERSE_N1_CPUACTLR_EL1_BIT_6
	msr	NEOVERSE_N1_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_n1_1073348_wa

func check_errata_1073348
	/* Applies to r0p0 and r1p0 */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1073348

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1130799
 * This applies to revision <=r2p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1130799_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_1130799
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUACTLR2_EL1
	orr	x1, x1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_59
	msr	NEOVERSE_N1_CPUACTLR2_EL1, x1
1:
	ret	x17
endfunc errata_n1_1130799_wa

func check_errata_1130799
	/* Applies to <=r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_1130799

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1165347
 * This applies to revision <=r2p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1165347_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_1165347
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUACTLR2_EL1
	orr	x1, x1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_0
	orr	x1, x1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_15
	msr	NEOVERSE_N1_CPUACTLR2_EL1, x1
1:
	ret	x17
endfunc errata_n1_1165347_wa

func check_errata_1165347
	/* Applies to <=r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_1165347

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1207823
 * This applies to revision <=r2p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1207823_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_1207823
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUACTLR2_EL1
	orr	x1, x1, NEOVERSE_N1_CPUACTLR2_EL1_BIT_11
	msr	NEOVERSE_N1_CPUACTLR2_EL1, x1
1:
	ret	x17
endfunc errata_n1_1207823_wa

func check_errata_1207823
	/* Applies to <=r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_1207823

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1220197
 * This applies to revision <=r2p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1220197_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_1220197
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUECTLR_EL1
	orr	x1, x1, NEOVERSE_N1_WS_THR_L2_MASK
	msr	NEOVERSE_N1_CPUECTLR_EL1, x1
1:
	ret	x17
endfunc errata_n1_1220197_wa

func check_errata_1220197
	/* Applies to <=r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_1220197

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1257314
 * This applies to revision <=r3p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1257314_wa
	/* Compare x0 against revision r3p0 */
	mov	x17, x30
	bl	check_errata_1257314
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUACTLR3_EL1
	orr	x1, x1, NEOVERSE_N1_CPUACTLR3_EL1_BIT_10
	msr	NEOVERSE_N1_CPUACTLR3_EL1, x1
1:
	ret	x17
endfunc errata_n1_1257314_wa

func check_errata_1257314
	/* Applies to <=r3p0 */
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1257314

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1262606
 * This applies to revision <=r3p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1262606_wa
	/* Compare x0 against revision r3p0 */
	mov	x17, x30
	bl	check_errata_1262606
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUACTLR_EL1
	orr	x1, x1, NEOVERSE_N1_CPUACTLR_EL1_BIT_13
	msr	NEOVERSE_N1_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_n1_1262606_wa

func check_errata_1262606
	/* Applies to <=r3p0 */
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1262606

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1262888
 * This applies to revision <=r3p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1262888_wa
	/* Compare x0 against revision r3p0 */
	mov	x17, x30
	bl	check_errata_1262888
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUECTLR_EL1
	orr	x1, x1, NEOVERSE_N1_CPUECTLR_EL1_MM_TLBPF_DIS_BIT
	msr	NEOVERSE_N1_CPUECTLR_EL1, x1
1:
	ret	x17
endfunc errata_n1_1262888_wa

func check_errata_1262888
	/* Applies to <=r3p0 */
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1262888

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata #1275112
 * This applies to revision <=r3p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1275112_wa
	/* Compare x0 against revision r3p0 */
	mov	x17, x30
	bl	check_errata_1275112
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUACTLR_EL1
	orr	x1, x1, NEOVERSE_N1_CPUACTLR_EL1_BIT_13
	msr	NEOVERSE_N1_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_n1_1275112_wa

func check_errata_1275112
	/* Applies to <=r3p0 */
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1275112

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Erratum 1315703.
 * This applies to revision <= r3p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1315703_wa
	/* Compare x0 against revision r3p1 */
	mov	x17, x30
	bl	check_errata_1315703
	cbz	x0, 1f

	mrs	x0, NEOVERSE_N1_CPUACTLR2_EL1
	orr	x0, x0, #NEOVERSE_N1_CPUACTLR2_EL1_BIT_16
	msr	NEOVERSE_N1_CPUACTLR2_EL1, x0

1:
	ret	x17
endfunc errata_n1_1315703_wa

func check_errata_1315703
	/* Applies to everything <= r3p0. */
	mov	x1, #0x30
	b	cpu_rev_var_ls
endfunc check_errata_1315703

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Erratum 1542419.
 * This applies to revisions r3p0 - r4p0 of Neoverse N1
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1542419_wa
	/* Compare x0 against revision r3p0 and r4p0 */
	mov	x17, x30
	bl	check_errata_1542419
	cbz	x0, 1f

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
1:
	ret	x17
endfunc errata_n1_1542419_wa

func check_errata_1542419
	/* Applies to everything r3p0 - r4p0. */
	mov	x1, #0x30
	mov	x2, #0x40
	b	cpu_rev_var_range
endfunc check_errata_1542419

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse N1 Errata #1868343.
	 * This applies to revision <= r4p0 of Neoverse N1.
	 * This workaround is the same as the workaround for
	 * errata 1262606 and 1275112 but applies to a wider
	 * revision range.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_n1_1868343_wa
	/*
	 * Compare x0 against revision r4p0
	 */
	mov	x17, x30
	bl	check_errata_1868343
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N1_CPUACTLR_EL1
	orr	x1, x1, NEOVERSE_N1_CPUACTLR_EL1_BIT_13
	msr	NEOVERSE_N1_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_n1_1868343_wa

func check_errata_1868343
	/* Applies to everything <= r4p0 */
	mov	x1, #0x40
	b	cpu_rev_var_ls
endfunc check_errata_1868343

func neoverse_n1_reset_func
	mov	x19, x30

	bl neoverse_n1_disable_speculative_loads

	/* Forces all cacheable atomic instructions to be near */
	mrs	x0, NEOVERSE_N1_CPUACTLR2_EL1
	orr	x0, x0, #NEOVERSE_N1_CPUACTLR2_EL1_BIT_2
	msr	NEOVERSE_N1_CPUACTLR2_EL1, x0
	isb

	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_N1_1043202
	mov	x0, x18
	bl	errata_n1_1043202_wa
#endif

#if ERRATA_N1_1073348
	mov	x0, x18
	bl	errata_n1_1073348_wa
#endif

#if ERRATA_N1_1130799
	mov	x0, x18
	bl	errata_n1_1130799_wa
#endif

#if ERRATA_N1_1165347
	mov	x0, x18
	bl	errata_n1_1165347_wa
#endif

#if ERRATA_N1_1207823
	mov	x0, x18
	bl	errata_n1_1207823_wa
#endif

#if ERRATA_N1_1220197
	mov	x0, x18
	bl	errata_n1_1220197_wa
#endif

#if ERRATA_N1_1257314
	mov	x0, x18
	bl	errata_n1_1257314_wa
#endif

#if ERRATA_N1_1262606
	mov	x0, x18
	bl	errata_n1_1262606_wa
#endif

#if ERRATA_N1_1262888
	mov	x0, x18
	bl	errata_n1_1262888_wa
#endif

#if ERRATA_N1_1275112
	mov	x0, x18
	bl	errata_n1_1275112_wa
#endif

#if ERRATA_N1_1315703
	mov	x0, x18
	bl	errata_n1_1315703_wa
#endif

#if ERRATA_N1_1542419
	mov	x0, x18
	bl	errata_n1_1542419_wa
#endif

#if ERRATA_N1_1868343
	mov	x0, x18
	bl	errata_n1_1868343_wa
#endif

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	orr	x0, x0, #NEOVERSE_N1_ACTLR_AMEN_BIT
	msr	actlr_el3, x0

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	orr	x0, x0, #NEOVERSE_N1_ACTLR_AMEN_BIT
	msr	actlr_el2, x0

	/* Enable group0 counters */
	mov	x0, #NEOVERSE_N1_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
#endif

#if NEOVERSE_N1_EXTERNAL_LLC
	/* Some system may have External LLC, core needs to be made aware */
	mrs     x0, NEOVERSE_N1_CPUECTLR_EL1
	orr     x0, x0, NEOVERSE_N1_CPUECTLR_EL1_EXTLLC_BIT
	msr     NEOVERSE_N1_CPUECTLR_EL1, x0
#endif

#if ERRATA_DSU_936184
	bl	errata_dsu_936184_wa
#endif

	isb
	ret	x19
endfunc neoverse_n1_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_n1_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, NEOVERSE_N1_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_N1_CORE_PWRDN_EN_MASK
	msr	NEOVERSE_N1_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_n1_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Neoverse N1. Must follow AAPCS.
 */
func neoverse_n1_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_N1_1043202, neoverse_n1, 1043202
	report_errata ERRATA_N1_1073348, neoverse_n1, 1073348
	report_errata ERRATA_N1_1130799, neoverse_n1, 1130799
	report_errata ERRATA_N1_1165347, neoverse_n1, 1165347
	report_errata ERRATA_N1_1207823, neoverse_n1, 1207823
	report_errata ERRATA_N1_1220197, neoverse_n1, 1220197
	report_errata ERRATA_N1_1257314, neoverse_n1, 1257314
	report_errata ERRATA_N1_1262606, neoverse_n1, 1262606
	report_errata ERRATA_N1_1262888, neoverse_n1, 1262888
	report_errata ERRATA_N1_1275112, neoverse_n1, 1275112
	report_errata ERRATA_N1_1315703, neoverse_n1, 1315703
	report_errata ERRATA_N1_1542419, neoverse_n1, 1542419
	report_errata ERRATA_N1_1868343, neoverse_n1, 1868343
	report_errata ERRATA_DSU_936184, neoverse_n1, dsu_936184

	ldp	x8, x30, [sp], #16
	ret
endfunc neoverse_n1_errata_report
#endif

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

#if IMAGE_BL31 && RAS_EXTENSION
	/*
	 * Issue Error Synchronization Barrier to synchronize SErrors before
	 * exiting EL3. We're running with EAs unmasked, so any synchronized
	 * errors would be taken immediately; therefore no need to inspect
	 * DISR_EL1 register.
	 */
	esb
#endif
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
