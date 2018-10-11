/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>
#include <cortex_a75.h>
#include <cortex_a55.h>
#include <cpu_macros.S>

	.globl	plat_arm_calc_core_pos
	.globl	plat_reset_handler

	/* ---------------------------------------------------------------------
	 * unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *
	 * Function to calculate the core position on FVP.
	 *
	 * (ClusterId * MAX_CPUS_PER_CLUSTER * MAX_PE_PER_CPU) +
	 * (CPUId * MAX_PE_PER_CPU) +
	 * ThreadId
	 *
	 * which can be simplified as:
	 *
	 * ((ClusterId * MAX_CPUS_PER_CLUSTER + CPUId) * MAX_PE_PER_CPU)
	 * + ThreadId
	 * ---------------------------------------------------------------------
	 */
func plat_arm_calc_core_pos
	/*
	 * Check for MT bit in MPIDR. If not set, shift MPIDR to left to make it
	 * look as if in a multi-threaded implementation.
	 */
	tst	x0, #MPIDR_MT_MASK
	lsr	x3, x0, #MPIDR_AFFINITY_BITS
	csel	x3, x3, x0, eq

	/* Extract individual affinity fields from MPIDR */
	ubfx	x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x2, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS

	/* Compute linear position */
	mov	x4, #PLAT_MAX_CPUS_PER_CLUSTER
	madd	x1, x2, x4, x1
	mov	x5, #PLAT_MAX_PE_PER_CPU
	madd	x0, x1, x5, x0
	ret
endfunc plat_arm_calc_core_pos

	/* -----------------------------------------------------
	 * void plat_reset_handler(void);
	 *
	 * Determine the CPU MIDR and disable power down bit for
	 * that CPU.
	 * -----------------------------------------------------
	 */
func plat_reset_handler
	jump_if_cpu_midr CORTEX_A75_MIDR, A75
	jump_if_cpu_midr CORTEX_A55_MIDR, A55
	ret

	/* -----------------------------------------------------
	 * Disable CPU power down bit in power control register
	 * -----------------------------------------------------
	 */
A75:
	mrs	x0, CORTEX_A75_CPUPWRCTLR_EL1
	bic	x0, x0, #CORTEX_A75_CORE_PWRDN_EN_MASK
	msr	CORTEX_A75_CPUPWRCTLR_EL1, x0
	isb
	ret
A55:
	mrs	x0, CORTEX_A55_CPUPWRCTLR_EL1
	bic	x0, x0, #CORTEX_A55_CORE_PWRDN_EN_MASK
	msr	CORTEX_A55_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc plat_reset_handler
