/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl plat_is_my_cpu_primary
	.globl plat_my_core_pos
	.globl plat_mediatek_calc_core_pos

func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 *  This function uses the plat_mediatek_calc_core_pos()
	 *  definition to get the index of the calling CPU.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	plat_mediatek_calc_core_pos
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * unsigned int plat_mediatek_calc_core_pos(u_register_t mpidr);
	 *
	 * In ARMv8.2, AFF2 is cluster id, AFF1 is core id and
	 * AFF0 is thread id. There is only one cluster in ARMv8.2
	 * and one thread in current implementation.
	 *
	 * With this function: CorePos = CoreID (AFF1)
	 * we do it with x0 = (x0 >> 8) & 0xff
	 * -----------------------------------------------------
	 */
func plat_mediatek_calc_core_pos
	mov	x1, #MPIDR_AFFLVL_MASK
	and	x0, x1, x0, lsr #MPIDR_AFF1_SHIFT
	ret
endfunc plat_mediatek_calc_core_pos
