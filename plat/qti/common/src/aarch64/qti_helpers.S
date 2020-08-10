/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/gicv3.h>
#include <drivers/console.h>

#include <platform_def.h>

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
	/* save the lr */
	mov	x18, x30

	/* Serialize CPUSS boot setup. Multi core enter simultaneously. */
	ldr	x0, =g_qti_cpuss_boot_lock
	bl	spin_lock

	/* pass cold boot status. */
	ldr	w0, g_qti_bl31_cold_booted
	/* Execuete CPUSS boot set up on every core. */
	bl	qtiseclib_cpuss_reset_asm

	ldr	x0, =g_qti_cpuss_boot_lock
	bl	spin_unlock

	ret	x18
endfunc plat_reset_handler
