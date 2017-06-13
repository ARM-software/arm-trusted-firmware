/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <platform_def.h>

	.global	uniphier_calc_core_pos
	.global	plat_my_core_pos
	.globl	platform_mem_init

/*
 * unsigned int uniphier_calc_core_pos(u_register_t mpidr)
 * core_pos = (cluster_id * max_cpus_per_cluster) + core_id
 */
func uniphier_calc_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	lsr	x0, x0, #MPIDR_AFFINITY_BITS
	mov	x2, #UNIPHIER_MAX_CPUS_PER_CLUSTER
	madd	x0, x0, x2, x1
	ret
endfunc uniphier_calc_core_pos

func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	uniphier_calc_core_pos
endfunc plat_my_core_pos

func platform_mem_init
	ret
endfunc platform_mem_init
