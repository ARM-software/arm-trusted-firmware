/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cpu_data.h>

.globl	init_cpu_data_ptr
.globl	_cpu_data_by_index

/* -----------------------------------------------------------------
 * void init_cpu_data_ptr(void)
 *
 * Initialise the TPIDR_EL3 register to refer to the cpu_data_t
 * for the calling CPU. This must be called before cm_get_cpu_data()
 *
 * This can be called without a valid stack. It assumes that
 * plat_my_core_pos() does not clobber register x10.
 * clobbers: x0, x1, x10
 * -----------------------------------------------------------------
 */
func init_cpu_data_ptr
	mov	x10, x30
	bl	plat_my_core_pos
	bl	_cpu_data_by_index
	msr	tpidr_el3, x0
	ret	x10
endfunc init_cpu_data_ptr

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
	adr	x1, percpu_data
	add	x0, x0, x1
	ret
endfunc _cpu_data_by_index
