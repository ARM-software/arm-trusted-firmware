/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.weak	plat_get_my_stack
	.weak	plat_set_my_stack

	/* -----------------------------------------------------
	 * uintptr_t plat_get_my_stack (u_register_t mpidr)
	 *
	 * For a given CPU, this function returns the stack
	 * pointer for a stack allocated in device memory.
	 * -----------------------------------------------------
	 */
func plat_get_my_stack
	push	{r4, lr}
	get_my_mp_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	pop	{r4, pc}
endfunc	plat_get_my_stack

	/* -----------------------------------------------------
	 * void plat_set_my_stack ()
	 *
	 * For the current CPU, this function sets the stack
	 * pointer to a stack allocated in normal memory.
	 * -----------------------------------------------------
	 */
func plat_set_my_stack
	mov	r4, lr
	get_my_mp_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	mov	sp, r0
	bx	r4
endfunc plat_set_my_stack

	/* -----------------------------------------------------
	 * Per-cpu stacks in normal memory. Each cpu gets a
	 * stack of PLATFORM_STACK_SIZE bytes.
	 * -----------------------------------------------------
	 */
declare_stack platform_normal_stacks, tzfw_normal_stacks, \
		PLATFORM_STACK_SIZE, PLATFORM_CORE_COUNT
