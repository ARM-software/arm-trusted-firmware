/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_is_my_cpu_primary
	.globl	plat_arm_calc_core_pos

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary
	 * cpu (applicable only after a cold boot)
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mov	x9, x30
	bl	plat_my_core_pos
	ldr	x1, =SGI_BOOT_CFG_ADDR
	ldr	x1, [x1]
	ubfx	x1, x1, #PLAT_CSS_PRIMARY_CPU_SHIFT, \
			#PLAT_CSS_PRIMARY_CPU_BIT_WIDTH
	cmp	x0, x1
	cset	w0, eq
	ret	x9
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 * unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *
	 * Helper function to calculate the core position.
	 * (ClusterId * CSS_SGI_MAX_CPUS_PER_CLUSTER * CSS_SGI_MAX_PE_PER_CPU) +
	 * (CPUId * CSS_SGI_MAX_PE_PER_CPU) +
	 * ThreadId
	 *
	 * which can be simplified as:
	 *
	 * ((ClusterId * CSS_SGI_MAX_CPUS_PER_CLUSTER + CPUId) *
	 * CSS_SGI_MAX_PE_PER_CPU) + ThreadId
	 * ------------------------------------------------------
	 */

func plat_arm_calc_core_pos
	mov	x3, x0

	/*
	 * The MT bit in MPIDR is always set for SGI platforms
	 * and the affinity level 0 corresponds to thread affinity level.
	 */

	/* Extract individual affinity fields from MPIDR */
	ubfx    x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx    x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx    x2, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS

	/* Compute linear position */
	mov     x4, #CSS_SGI_MAX_CPUS_PER_CLUSTER
	madd    x1, x2, x4, x1
	mov     x5, #CSS_SGI_MAX_PE_PER_CPU
	madd    x0, x1, x5, x0
	ret
endfunc plat_arm_calc_core_pos
