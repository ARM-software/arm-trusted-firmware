/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>

	.globl	plat_my_core_pos
	.globl	plat_is_my_cpu_primary
	.globl	plat_get_my_entrypoint
	.weak	platform_get_core_pos

	/* -----------------------------------------------------
	 *  Compatibility wrappers for new platform APIs.
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	b	platform_get_core_pos
endfunc plat_my_core_pos

func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	b	platform_is_primary_cpu
endfunc plat_is_my_cpu_primary

func plat_get_my_entrypoint
	mrs	x0, mpidr_el1
	b	platform_get_entrypoint
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 *  int platform_get_core_pos(int mpidr);
	 *  With this function: CorePos = (ClusterId * 4) +
	 *                                CoreId
	 * -----------------------------------------------------
	 */
func platform_get_core_pos
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc platform_get_core_pos
