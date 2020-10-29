/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>

	.local	platform_normal_stacks
	.weak	plat_get_my_stack
	.weak	plat_set_my_stack

	/* ---------------------------------------------------------------------
	 * When the compatility layer is disabled, the platform APIs
	 * plat_get_my_stack() and plat_set_my_stack() are supported by the
	 * platform and the previous APIs platform_get_stack() and
	 * platform_set_stack() are defined in terms of new APIs making use of
	 * the fact that they are only ever invoked for the current CPU.  This
	 * is to enable components of Trusted Firmware like SPDs using the old
	 * platform APIs to continue to work.
	 * --------------------------------------------------------------------
	 */

	/* -----------------------------------------------------
	 * uintptr_t plat_get_my_stack ()
	 *
	 * For the current CPU, this function returns the stack
	 * pointer for a stack allocated in device memory.
	 * -----------------------------------------------------
	 */
func plat_get_my_stack
	mov	x10, x30
	get_my_mp_stack platform_normal_stacks, PLATFORM_STACK_SIZE
	ret	x10
endfunc plat_get_my_stack

	/* -----------------------------------------------------
	 * void plat_set_my_stack ()
	 *
	 * For the current CPU, this function sets the stack
	 * pointer to a stack allocated in normal memory.
	 * -----------------------------------------------------
	 */
func plat_set_my_stack
	mov	x9, x30
	bl 	plat_get_my_stack
	mov	sp, x0
	ret	x9
endfunc plat_set_my_stack

	/* -----------------------------------------------------
	 * Per-CPU stacks in normal memory. Each CPU gets a
	 * stack of PLATFORM_STACK_SIZE bytes.
	 * -----------------------------------------------------
	 */
declare_stack platform_normal_stacks, tzfw_normal_stacks, \
		PLATFORM_STACK_SIZE, PLATFORM_CORE_COUNT, \
		CACHE_WRITEBACK_GRANULE
