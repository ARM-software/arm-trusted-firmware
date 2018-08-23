/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <gicv2.h>
#include <gicv3.h>
#include <platform_def.h>
#include <console.h>

#define PER_CPU_COHERENT_STACK_SIZE  0x100

	.globl	plat_my_core_pos
	.globl	plat_qti_core_pos_by_mpidr
	.globl	plat_reset_handler
	.globl	plat_panic_handler

	/* -----------------------------------------------------
	 *  unsigned int plat_qti_core_pos_by_mpidr(uint64_t mpidr)
	 *  Helper function to calculate the core position.
	 *  With this function:
	 *  CorePos = (ClusterId * 4) + CoreId
	 *  - In ARM v8   (MPIDR_EL1[24]=0)
	 *    ClusterId = MPIDR_EL1[15:8]
	 *    CoreId    = MPIDR_EL1[7:0]
	 *  - In ARM v8.1 (MPIDR_EL1[24]=1)
	 *    ClusterId = MPIDR_EL1[23:15]
	 *    CoreId    = MPIDR_EL1[15:8]
	 *  Clobbers: x0 & x1.
	 * -----------------------------------------------------
	 */
func plat_qti_core_pos_by_mpidr
	mrs	x1, mpidr_el1
	tst	x1, #MPIDR_MT_MASK
	beq	plat_qti_core_pos_by_mpidr_no_mt
	/* Right shift mpidr by one affinity level when MT=1. */
	lsr	x0, x0, #MPIDR_AFFINITY_BITS
plat_qti_core_pos_by_mpidr_no_mt:
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_qti_core_pos_by_mpidr

	/* --------------------------------------------------------------------
	 * void plat_panic_handler(void)
	 * calls SDI and reset system
	 * --------------------------------------------------------------------
	 */
func plat_panic_handler
	msr	spsel, #0
	bl	plat_set_my_stack
	b	qtiseclib_panic
endfunc plat_panic_handler

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_qti_calc_core_pos()
	 *  definition to get the index of the calling CPU
	 *  Clobbers: x0 & x1.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_qti_core_pos_by_mpidr
endfunc plat_my_core_pos

func plat_reset_handler
	mov	x18, x30	// save the lr
	bl	plat_my_core_pos
	add	x1, x0, #1	// x1 now contains logical-core-id + 1: Helps to calculate top of stack
	mov	x30, x18

	ldr	x0, =per_cpu_coherent_boot_stacks
	mov	x2, PER_CPU_COHERENT_STACK_SIZE
	mul	x1, x1, x2
	add	x0, x0, x1	// each core gets PER_CPU_COHERENT_STACK_SIZE of stack area
	mov	sp, x0

	stp	x29, x30, [sp,#-16]!
	stp	x27, x28, [sp,#-16]!
	stp	x25, x26, [sp,#-16]!
	stp	x23, x24, [sp,#-16]!
	stp	x21, x22, [sp,#-16]!
	stp	x19, x20, [sp,#-16]!
	bl	qtiseclib_cpuss_boot_setup
	ldp	x19, x20, [sp],#16
	ldp	x21, x22, [sp],#16
	ldp	x23, x24, [sp],#16
	ldp	x25, x26, [sp],#16
	ldp	x27, x28, [sp],#16
	ldp	x29, x30, [sp],#16
	ret
endfunc plat_reset_handler


	.p2align 3
cold_boot_done:
	.quad 0x0

	/* ----------------------------------------------------
	 * Per cpu boot stack in coherent memory.
	 * ----------------------------------------------------
	 */
declare_stack per_cpu_coherent_boot_stacks, tzfw_coherent_mem, \
		PER_CPU_COHERENT_STACK_SIZE, PLATFORM_CORE_COUNT, CACHE_WRITEBACK_GRANULE
