/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include "opteed_private.h"

	.global	opteed_enter_sp
	/* ---------------------------------------------
	 * This function is called with SP_EL0 as stack.
	 * Here we stash our EL3 callee-saved registers
	 * on to the stack as a part of saving the C
	 * runtime and enter the secure payload.
	 * 'x0' contains a pointer to the memory where
	 * the address of the C runtime context is to be
	 * saved.
	 * ---------------------------------------------
	 */
func opteed_enter_sp
	/* Make space for the registers that we're going to save */
	mov	x3, sp
	str	x3, [x0, #0]
	sub	sp, sp, #OPTEED_C_RT_CTX_SIZE

	/* Save callee-saved registers on to the stack */
	stp	x19, x20, [sp, #OPTEED_C_RT_CTX_X19]
	stp	x21, x22, [sp, #OPTEED_C_RT_CTX_X21]
	stp	x23, x24, [sp, #OPTEED_C_RT_CTX_X23]
	stp	x25, x26, [sp, #OPTEED_C_RT_CTX_X25]
	stp	x27, x28, [sp, #OPTEED_C_RT_CTX_X27]
	stp	x29, x30, [sp, #OPTEED_C_RT_CTX_X29]

	/* ---------------------------------------------
	 * Everything is setup now. el3_exit() will
	 * use the secure context to restore to the
	 * general purpose and EL3 system registers to
	 * ERET into OPTEE.
	 * ---------------------------------------------
	 */
	b	el3_exit
endfunc opteed_enter_sp

	/* ---------------------------------------------
	 * This function is called 'x0' pointing to a C
	 * runtime context saved in opteed_enter_sp().  It
	 * restores the saved registers and jumps to
	 * that runtime with 'x0' as the new sp. This
	 * destroys the C runtime context that had been
	 * built on the stack below the saved context by
	 * the caller. Later the second parameter 'x1'
	 * is passed as return value to the caller
	 * ---------------------------------------------
	 */
	.global opteed_exit_sp
func opteed_exit_sp
	/* Restore the previous stack */
	mov	sp, x0

	/* Restore callee-saved registers on to the stack */
	ldp	x19, x20, [x0, #(OPTEED_C_RT_CTX_X19 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x21, x22, [x0, #(OPTEED_C_RT_CTX_X21 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x23, x24, [x0, #(OPTEED_C_RT_CTX_X23 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x25, x26, [x0, #(OPTEED_C_RT_CTX_X25 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x27, x28, [x0, #(OPTEED_C_RT_CTX_X27 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x29, x30, [x0, #(OPTEED_C_RT_CTX_X29 - OPTEED_C_RT_CTX_SIZE)]

	/* ---------------------------------------------
	 * This should take us back to the instruction
	 * after the call to the last opteed_enter_sp().
	 * Place the second parameter to x0 so that the
	 * caller will see it as a return value from the
	 * original entry call
	 * ---------------------------------------------
	 */
	mov	x0, x1
	ret
endfunc opteed_exit_sp
