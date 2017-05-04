/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>


	.local	platform_normal_stacks
	.weak	plat_set_my_stack
	.weak	plat_get_my_stack
	.weak	platform_set_stack
	.weak	platform_get_stack

	/* -----------------------------------------------------
	 * uintptr_t plat_get_my_stack ()
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function returns the stack pointer for a
	 * stack allocated in device memory.
	 * -----------------------------------------------------
	 */
func plat_get_my_stack
	get_up_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	ret
endfunc plat_get_my_stack

	/* -----------------------------------------------------
	 * void plat_set_my_stack ()
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function sets the stack pointer to a stack
	 * allocated in normal memory.
	 * -----------------------------------------------------
	 */
func plat_set_my_stack
	get_up_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	mov sp, x0
	ret
endfunc plat_set_my_stack

	/* -----------------------------------------------------
	 * unsigned long platform_get_stack ()
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function returns the stack pointer for a
	 * stack allocated in device memory. This function
	 * is deprecated.
	 * -----------------------------------------------------
	 */
func_deprecated platform_get_stack
	b	plat_get_my_stack
endfunc_deprecated platform_get_stack

	/* -----------------------------------------------------
	 * void platform_set_stack ()
	 *
	 * For cold-boot BL images, only the primary CPU needs a
	 * stack. This function sets the stack pointer to a stack
	 * allocated in normal memory.This function is
	 * deprecated.
	 * -----------------------------------------------------
	 */
func_deprecated platform_set_stack
	b	plat_set_my_stack
endfunc_deprecated platform_set_stack

	/* -----------------------------------------------------
	 * Single cpu stack in normal memory.
	 * Used for C code during boot, PLATFORM_STACK_SIZE bytes
	 * are allocated
	 * -----------------------------------------------------
	 */
declare_stack platform_normal_stacks, tzfw_normal_stacks, \
		PLATFORM_STACK_SIZE, 1, CACHE_WRITEBACK_GRANULE
