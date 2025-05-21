/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <lib/el3_runtime/cpu_data.h>

.globl	_cpu_data_by_index

/* -----------------------------------------------------------------
 * cpu_data_t *_cpu_data_by_index(uint32_t cpu_index)
 *
 * Return the cpu_data structure for the CPU with given linear index
 *
 * This can be called without a valid stack.
 * clobbers: x0, x1
 * -----------------------------------------------------------------
 */
func _cpu_data_by_index
	mov_imm	x1, CPU_DATA_SIZE
	mul	x0, x0, x1
	adrp	x1, percpu_data
	add	x1, x1, :lo12:percpu_data
	add	x0, x0, x1
	ret
endfunc _cpu_data_by_index
