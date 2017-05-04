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
	 * unsigned long plat_get_my_stack ()
	 *
	 * For cold-boot BL images, only the primary CPU needs
	 * a stack. This function returns the stack pointer for
	 * a stack allocated in normal memory.
	 * -----------------------------------------------------
	 */
func plat_get_my_stack
	get_up_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	bx	lr
endfunc	plat_get_my_stack

	/* -----------------------------------------------------
	 * void plat_set_my_stack ()
	 *
	 * For cold-boot BL images, only the primary CPU needs
	 * a stack. This function sets the stack pointer to a
	 * stack allocated in normal memory.
	 * -----------------------------------------------------
	 */
func plat_set_my_stack
	get_up_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	mov	sp, r0
	bx	lr
endfunc plat_set_my_stack

	/* -----------------------------------------------------
	 * Per-cpu stacks in normal memory. Each cpu gets a
	 * stack of PLATFORM_STACK_SIZE bytes.
	 * -----------------------------------------------------
	 */
declare_stack platform_normal_stacks, tzfw_normal_stacks, \
		PLATFORM_STACK_SIZE, 1, CACHE_WRITEBACK_GRANULE
