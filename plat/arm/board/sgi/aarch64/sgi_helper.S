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
	*  unsigned int plat_arm_calc_core_pos(uint64_t mpidr)
	*  Helper function to calculate the core position.
	* -----------------------------------------------------
	*/
func plat_arm_calc_core_pos
	mrs     x2, mpidr_el1
	ands    x2, x2, #MPIDR_MT_MASK
	beq     1f
	lsr     x0, x0, #MPIDR_AFF1_SHIFT
1:
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	and     x0, x0, #MPIDR_AFFLVL_MASK
	ret
endfunc plat_arm_calc_core_pos
