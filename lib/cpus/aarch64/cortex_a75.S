/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include <cortex_a75.h>

	.globl	cortex_a75_amu_cnt_read
	.globl	cortex_a75_amu_cnt_write
	.globl	cortex_a75_amu_read_cpuamcntenset_el0
	.globl	cortex_a75_amu_read_cpuamcntenclr_el0
	.globl	cortex_a75_amu_write_cpuamcntenset_el0
	.globl	cortex_a75_amu_write_cpuamcntenclr_el0

/*
 * uint64_t cortex_a75_amu_cnt_read(int idx);
 *
 * Given `idx`, read the corresponding AMU counter
 * and return it in `x0`.
 */
func cortex_a75_amu_cnt_read
	adr	x1, 1f
	lsl	x0, x0, #3
	add	x1, x1, x0
	br	x1

1:
	mrs	x0, CPUAMEVCNTR0_EL0
	ret
	mrs	x0, CPUAMEVCNTR1_EL0
	ret
	mrs	x0, CPUAMEVCNTR2_EL0
	ret
	mrs	x0, CPUAMEVCNTR3_EL0
	ret
	mrs	x0, CPUAMEVCNTR4_EL0
	ret
endfunc cortex_a75_amu_cnt_read

/*
 * void cortex_a75_amu_cnt_write(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU counter.
 */
func cortex_a75_amu_cnt_write
	adr	x2, 1f
	lsl	x0, x0, #3
	add	x2, x2, x0
	br	x2

1:
	msr	CPUAMEVCNTR0_EL0, x0
	ret
	msr	CPUAMEVCNTR1_EL0, x0
	ret
	msr	CPUAMEVCNTR2_EL0, x0
	ret
	msr	CPUAMEVCNTR3_EL0, x0
	ret
	msr	CPUAMEVCNTR4_EL0, x0
	ret
endfunc cortex_a75_amu_cnt_write

/*
 * unsigned int cortex_a75_amu_read_cpuamcntenset_el0(void);
 *
 * Read the `CPUAMCNTENSET_EL0` CPU register and return
 * it in `x0`.
 */
func cortex_a75_amu_read_cpuamcntenset_el0
	mrs	x0, CPUAMCNTENSET_EL0
	ret
endfunc cortex_a75_amu_read_cpuamcntenset_el0

/*
 * unsigned int cortex_a75_amu_read_cpuamcntenclr_el0(void);
 *
 * Read the `CPUAMCNTENCLR_EL0` CPU register and return
 * it in `x0`.
 */
func cortex_a75_amu_read_cpuamcntenclr_el0
	mrs	x0, CPUAMCNTENCLR_EL0
	ret
endfunc cortex_a75_amu_read_cpuamcntenclr_el0

/*
 * void cortex_a75_amu_write_cpuamcntenset_el0(unsigned int mask);
 *
 * Write `mask` to the `CPUAMCNTENSET_EL0` CPU register.
 */
func cortex_a75_amu_write_cpuamcntenset_el0
	msr	CPUAMCNTENSET_EL0, x0
	ret
endfunc cortex_a75_amu_write_cpuamcntenset_el0

/*
 * void cortex_a75_amu_write_cpuamcntenclr_el0(unsigned int mask);
 *
 * Write `mask` to the `CPUAMCNTENCLR_EL0` CPU register.
 */
func cortex_a75_amu_write_cpuamcntenclr_el0
	mrs	x0, CPUAMCNTENCLR_EL0
	ret
endfunc cortex_a75_amu_write_cpuamcntenclr_el0

func cortex_a75_reset_func
#if IMAGE_BL31 && WORKAROUND_CVE_2017_5715
	mrs	x0, id_aa64pfr0_el1
	ubfx	x0, x0, #ID_AA64PFR0_CSV2_SHIFT, #ID_AA64PFR0_CSV2_LENGTH
	/*
	 * If the field equals to 1 then branch targets trained in one
	 * context cannot affect speculative execution in a different context.
	 */
	cmp	x0, #1
	beq	1f

	adr	x0, workaround_bpiall_vbar0_runtime_exceptions
	msr	vbar_el3, x0
1:
#endif

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	orr	x0, x0, #CORTEX_A75_ACTLR_AMEN_BIT
	msr	actlr_el3, x0
	isb

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	orr	x0, x0, #CORTEX_A75_ACTLR_AMEN_BIT
	msr	actlr_el2, x0
	isb

	/* Enable group0 counters */
	mov	x0, #CORTEX_A75_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
	isb

	/* Enable group1 counters */
	mov	x0, #CORTEX_A75_AMU_GROUP1_MASK
	msr	CPUAMCNTENSET_EL0, x0
	isb
#endif
	ret
endfunc cortex_a75_reset_func

func check_errata_cve_2017_5715
	mrs	x0, id_aa64pfr0_el1
	ubfx	x0, x0, #ID_AA64PFR0_CSV2_SHIFT, #ID_AA64PFR0_CSV2_LENGTH
	/*
	 * If the field equals to 1 then branch targets trained in one
	 * context cannot affect speculative execution in a different context.
	 */
	cmp	x0, #1
	beq	1f

#if WORKAROUND_CVE_2017_5715
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
1:
	mov	x0, #ERRATA_NOT_APPLIES
	ret
endfunc check_errata_cve_2017_5715

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a75_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A75_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A75_CORE_PWRDN_EN_MASK
	msr	CORTEX_A75_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a75_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A75. Must follow AAPCS.
 */
func cortex_a75_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata WORKAROUND_CVE_2017_5715, cortex_a75, cve_2017_5715

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a75_errata_report
#endif

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

declare_cpu_ops cortex_a75, CORTEX_A75_MIDR, \
	cortex_a75_reset_func, \
	cortex_a75_core_pwr_dwn
