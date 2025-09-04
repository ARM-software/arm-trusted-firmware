/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>
#include <cpu_macros.S>

#include <c1_pro.h>

#define TC_HANDLER(rev)         plat_reset_handler_tc##rev
#define PLAT_RESET_HANDLER(rev) TC_HANDLER(rev)

	.globl	plat_arm_calc_core_pos
	.globl	plat_reset_handler

	/* ---------------------------------------------------------------------
	 * unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
	 *
	 * Function to calculate the core position on TC.
	 *
	 * (ClusterId * PLAT_MAX_CPUS_PER_CLUSTER * PLAT_MAX_PE_PER_CPU) +
	 * (CPUId * PLAT_MAX_PE_PER_CPU) +
	 * ThreadId
	 *
	 * which can be simplified as:
	 *
	 * ((ClusterId * PLAT_MAX_CPUS_PER_CLUSTER + CPUId) * PLAT_MAX_PE_PER_CPU)
	 * + ThreadId
	 * ---------------------------------------------------------------------
	 */
func plat_arm_calc_core_pos
	/*
	 * Check for MT bit in MPIDR. If not set, shift MPIDR to left to make it
	 * look as if in a multi-threaded implementation.
	 */
	tst	x0, #MPIDR_MT_MASK
	lsl	x3, x0, #MPIDR_AFFINITY_BITS
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

func mark_extllc_presence
#ifdef MCN_CONFIG_ADDR
	mov_imm	x0, (MCN_CONFIG_ADDR(0))
	ldr	w1, [x0]
	ubfx	x1, x1, #MCN_CONFIG_SLC_PRESENT_BIT, #1
	jump_if_cpu_midr C1_PRO_MIDR, ARM_C1_PRO
	sysreg_bitfield_insert_from_gpr CPUECTLR_EL1, x1, CPUECTLR_EL1_EXTLLC_BIT, 1
	ret
ARM_C1_PRO:
	sysreg_bitfield_insert_from_gpr C1_PRO_IMP_CPUECTLR_EL1, x1, C1_PRO_CPUECTLR2_EL1_EXTLLC_BIT, 1
#endif
	ret
endfunc mark_extllc_presence

func enable_dsu_pmu_el1_access
	sysreg_bit_set actlr_el2, CPUACTLR_CLUSTERPMUEN
	sysreg_bit_set actlr_el3, CPUACTLR_CLUSTERPMUEN
	ret
endfunc enable_dsu_pmu_el1_access

func TC_HANDLER(3)
	mov	x9, lr
	bl	mark_extllc_presence
	bl	enable_dsu_pmu_el1_access
	mov	lr, x9
	ret
endfunc TC_HANDLER(3)

func TC_HANDLER(4)
	mov	x9, lr
	bl	mark_extllc_presence
	bl	enable_dsu_pmu_el1_access
	mov	lr, x9
	ret
endfunc TC_HANDLER(4)

	/* -----------------------------------------------------
	 * void plat_reset_handler(void);
	 * -----------------------------------------------------
	 */
func plat_reset_handler
	mov	x8, lr
	bl	PLAT_RESET_HANDLER(TARGET_PLATFORM)
	mov	lr, x8
	ret
endfunc plat_reset_handler
