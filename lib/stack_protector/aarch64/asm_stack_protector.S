/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>

	.globl	update_stack_protector_canary

/* -----------------------------------------------------------------------
 * void update_stack_protector_canary(void)
 *
 * Change the value of the canary used for stack smashing attacks protection.
 * Note: This must be called when it is safe to call C code, but this cannot be
 * called by C code. Doing this will make the check fail when the calling
 * function returns.
 * -----------------------------------------------------------------------
 */

func update_stack_protector_canary
	/* Use x19 as it is callee-saved */
	mov	x19, x30
	bl	plat_get_stack_protector_canary

	/* Update the canary with the returned value */
	adrp	x1,  __stack_chk_guard
	str	x0, [x1, #:lo12:__stack_chk_guard]
	ret	x19
endfunc update_stack_protector_canary


