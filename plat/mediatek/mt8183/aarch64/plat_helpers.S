/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl plat_report_exception
	.globl plat_is_my_cpu_primary
	.globl plat_my_core_pos
	.globl read_cpuectlr
	.globl write_cpuectlr

func plat_is_my_cpu_primary
	mrs x0, mpidr_el1
	and x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp x0, #PLAT_PRIMARY_CPU
	cset x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 * unsigned int plat_my_core_pos(void);
	 *
	 * result: CorePos = CoreId + (ClusterId << 2)
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs     x0, mpidr_el1
	and     x1, x0, #MPIDR_CPU_MASK
	and     x0, x0, #MPIDR_CLUSTER_MASK
	add     x0, x1, x0, LSR #6
	ret
endfunc plat_my_core_pos

func read_cpuectlr
	MRS     x0, S3_1_C15_C2_1
	ret
endfunc read_cpuectlr

func write_cpuectlr
	MSR     S3_1_C15_C2_1, x0
	ret
endfunc write_cpuectlr

