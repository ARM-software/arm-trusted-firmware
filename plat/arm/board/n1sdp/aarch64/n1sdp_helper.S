/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cortex_ares.h>
#include <cpu_macros.S>
#include <platform_def.h>

	.globl	plat_arm_calc_core_pos
	.globl	plat_reset_handler

	/* -----------------------------------------------------
	 * unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *
	 * Helper function to calculate the core position.
	 * (ClusterId * N1SDP_MAX_CPUS_PER_CLUSTER * N1SDP_MAX_PE_PER_CPU) +
	 * (CPUId * N1SDP_MAX_PE_PER_CPU) +
	 * ThreadId
	 *
	 * which can be simplified as:
	 *
	 * ((ClusterId * N1SDP_MAX_CPUS_PER_CLUSTER + CPUId) *
	 * N1SDP_MAX_PE_PER_CPU) + ThreadId
	 * ------------------------------------------------------
	 */

func plat_arm_calc_core_pos
	mov	x3, x0

	/*
	 * The MT bit in MPIDR is always set for n1sdp and the
	 * affinity level 0 corresponds to thread affinity level.
	 */

	/* Extract individual affinity fields from MPIDR */
	ubfx	x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx	x2, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS

	/* Compute linear position */
	mov	x4, #N1SDP_MAX_CPUS_PER_CLUSTER
	madd	x1, x2, x4, x1
	mov	x5, #N1SDP_MAX_PE_PER_CPU
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
	jump_if_cpu_midr CORTEX_ARES_MIDR, ARES
	ret

	/* -----------------------------------------------------
	 * Disable CPU power down bit in power control register
	 * -----------------------------------------------------
	 */
ARES:
	mrs	x0, CORTEX_ARES_CPUPWRCTLR_EL1
	bic	x0, x0, #CORTEX_ARES_CORE_PWRDN_EN_MASK
	msr	CORTEX_ARES_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc plat_reset_handler
