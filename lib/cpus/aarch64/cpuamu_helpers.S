/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpuamu.h>

	.globl	cpuamu_cnt_read
	.globl	cpuamu_cnt_write
	.globl	cpuamu_read_cpuamcntenset_el0
	.globl	cpuamu_read_cpuamcntenclr_el0
	.globl	cpuamu_write_cpuamcntenset_el0
	.globl	cpuamu_write_cpuamcntenclr_el0

/*
 * uint64_t cpuamu_cnt_read(int idx);
 *
 * Given `idx`, read the corresponding AMU counter
 * and return it in `x0`.
 */
func cpuamu_cnt_read
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
endfunc cpuamu_cnt_read

/*
 * void cpuamu_cnt_write(int idx, uint64_t val);
 *
 * Given `idx`, write `val` to the corresponding AMU counter.
 */
func cpuamu_cnt_write
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
endfunc cpuamu_cnt_write

/*
 * unsigned int cpuamu_read_cpuamcntenset_el0(void);
 *
 * Read the `CPUAMCNTENSET_EL0` CPU register and return
 * it in `x0`.
 */
func cpuamu_read_cpuamcntenset_el0
	mrs	x0, CPUAMCNTENSET_EL0
	ret
endfunc cpuamu_read_cpuamcntenset_el0

/*
 * unsigned int cpuamu_read_cpuamcntenclr_el0(void);
 *
 * Read the `CPUAMCNTENCLR_EL0` CPU register and return
 * it in `x0`.
 */
func cpuamu_read_cpuamcntenclr_el0
	mrs	x0, CPUAMCNTENCLR_EL0
	ret
endfunc cpuamu_read_cpuamcntenclr_el0

/*
 * void cpuamu_write_cpuamcntenset_el0(unsigned int mask);
 *
 * Write `mask` to the `CPUAMCNTENSET_EL0` CPU register.
 */
func cpuamu_write_cpuamcntenset_el0
	msr	CPUAMCNTENSET_EL0, x0
	ret
endfunc cpuamu_write_cpuamcntenset_el0

/*
 * void cpuamu_write_cpuamcntenclr_el0(unsigned int mask);
 *
 * Write `mask` to the `CPUAMCNTENCLR_EL0` CPU register.
 */
func cpuamu_write_cpuamcntenclr_el0
	msr	CPUAMCNTENCLR_EL0, x0
	ret
endfunc cpuamu_write_cpuamcntenclr_el0
