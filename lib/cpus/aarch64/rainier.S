/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <cpu_macros.S>
#include <cpuamu.h>
#include <rainier.h>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Rainier CPU must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Rainier CPU supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_RAINIER_IC_TRAP
	.global rainier_errata_ic_trap_handler
#endif

/* --------------------------------------------------
 * Disable speculative loads if Rainier supports
 * SSBS.
 *
 * Shall clobber: x0.
 * --------------------------------------------------
 */
func rainier_disable_speculative_loads
	/* Check if the PE implements SSBS */
	mrs	x0, id_aa64pfr1_el1
	tst	x0, #(ID_AA64PFR1_EL1_SSBS_MASK << ID_AA64PFR1_EL1_SSBS_SHIFT)
	b.eq	1f

	/* Disable speculative loads */
	msr	SSBS, xzr

1:
	ret
endfunc rainier_disable_speculative_loads

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Erratum 1542419.
 * This applies to revisions r3p0 - r4p0 of Neoverse N1
 * Since Rainier core is based on Neoverse N1 r4p0, this
 * errata applies to Rainier core r0p0
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
	mov	x0, xzr
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
	/* Applies to Rainier core r0p0. */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_1542419

func rainier_reset_func
	mov	x19, x30

	bl	rainier_disable_speculative_loads

	/* Forces all cacheable atomic instructions to be near */
	mrs	x0, RAINIER_CPUACTLR2_EL1
	orr	x0, x0, #RAINIER_CPUACTLR2_EL1_BIT_2
	msr	RAINIER_CPUACTLR2_EL1, x0
	isb

	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_N1_1542419
	mov	x0, x18
	bl	errata_n1_1542419_wa
#endif

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	orr	x0, x0, #RAINIER_ACTLR_AMEN_BIT
	msr	actlr_el3, x0

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	orr	x0, x0, #RAINIER_ACTLR_AMEN_BIT
	msr	actlr_el2, x0

	/* Enable group0 counters */
	mov	x0, #RAINIER_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
#endif

	isb
	ret	x19
endfunc rainier_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func rainier_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, RAINIER_CPUPWRCTLR_EL1
	orr	x0, x0, #RAINIER_CORE_PWRDN_EN_MASK
	msr	RAINIER_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc rainier_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Rainier. Must follow AAPCS.
 */
func rainier_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_N1_1542419, rainier, 1542419

	ldp	x8, x30, [sp], #16
	ret
endfunc rainier_errata_report
#endif

/*
 * Handle trap of EL0 IC IVAU instructions to EL3 by executing a TLB
 * inner-shareable invalidation to an arbitrary address followed by a DSB.
 *
 * x1: Exception Syndrome
 */
func rainier_errata_ic_trap_handler
	cmp	x1, #RAINIER_EC_IC_TRAP
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
	eret
1:
	ret
endfunc rainier_errata_ic_trap_handler

	/* ---------------------------------------------
	 * This function provides Rainier specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.rainier_regs, "aS"
rainier_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func rainier_cpu_reg_dump
	adr	x6, rainier_regs
	mrs	x8, RAINIER_CPUECTLR_EL1
	ret
endfunc rainier_cpu_reg_dump

declare_cpu_ops_eh rainier, RAINIER_MIDR, \
	rainier_reset_func, \
	rainier_errata_ic_trap_handler, \
	rainier_core_pwr_dwn
