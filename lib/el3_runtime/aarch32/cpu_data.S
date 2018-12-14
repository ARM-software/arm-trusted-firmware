/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <lib/el3_runtime/cpu_data.h>

	.globl	_cpu_data
	.globl	_cpu_data_by_index

/* -----------------------------------------------------------------
 * cpu_data_t *_cpu_data(void)
 *
 * Return the cpu_data structure for the current CPU.
 * -----------------------------------------------------------------
 */
func _cpu_data
	/* r12 is pushed to meet the 8 byte stack alignment requirement */
	push	{r12, lr}
	bl	plat_my_core_pos
	pop	{r12, lr}
	b	_cpu_data_by_index
endfunc _cpu_data

/* -----------------------------------------------------------------
 * cpu_data_t *_cpu_data_by_index(uint32_t cpu_index)
 *
 * Return the cpu_data structure for the CPU with given linear index
 *
 * This can be called without a valid stack.
 * clobbers: r0, r1
 * -----------------------------------------------------------------
 */
func _cpu_data_by_index
	mov_imm	r1, CPU_DATA_SIZE
	mul	r0, r0, r1
	ldr	r1, =percpu_data
	add	r0, r0, r1
	bx	lr
endfunc _cpu_data_by_index
