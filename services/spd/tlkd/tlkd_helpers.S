/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <asm_macros.S>
#include "tlkd_private.h"

	.global tlkd_enter_sp
	.global tlkd_exit_sp

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
func tlkd_enter_sp
	/* Make space for the registers that we're going to save */
	mov	x3, sp
	str	x3, [x0, #0]
	sub	sp, sp, #TLKD_C_RT_CTX_SIZE

	/* Save callee-saved registers on to the stack */
	stp	x19, x20, [sp, #TLKD_C_RT_CTX_X19]
	stp	x21, x22, [sp, #TLKD_C_RT_CTX_X21]
	stp	x23, x24, [sp, #TLKD_C_RT_CTX_X23]
	stp	x25, x26, [sp, #TLKD_C_RT_CTX_X25]
	stp	x27, x28, [sp, #TLKD_C_RT_CTX_X27]
	stp	x29, x30, [sp, #TLKD_C_RT_CTX_X29]

	/* ----------------------------------------------
	 * Everything is setup now. el3_exit() will
	 * use the secure context to restore to the
	 * general purpose and EL3 system registers to
	 * ERET into the secure payload.
	 * ----------------------------------------------
	 */
	b	el3_exit
endfunc tlkd_enter_sp

	/* ----------------------------------------------
	 * This function is called with 'x0' pointing to
	 * a C runtime context saved in tlkd_enter_sp().
	 * It restores the saved registers and jumps to
	 * that runtime with 'x0' as the new sp. This
	 * destroys the C runtime context that had been
	 * built on the stack below the saved context by
	 * the caller. Later the second parameter 'x1'
	 * is passed as return value to the caller
	 * ----------------------------------------------
	 */
func tlkd_exit_sp
	/* Restore the previous stack */
	mov	sp, x0

	/* Restore callee-saved registers on to the stack */
	ldp	x19, x20, [x0, #(TLKD_C_RT_CTX_X19 - TLKD_C_RT_CTX_SIZE)]
	ldp	x21, x22, [x0, #(TLKD_C_RT_CTX_X21 - TLKD_C_RT_CTX_SIZE)]
	ldp	x23, x24, [x0, #(TLKD_C_RT_CTX_X23 - TLKD_C_RT_CTX_SIZE)]
	ldp	x25, x26, [x0, #(TLKD_C_RT_CTX_X25 - TLKD_C_RT_CTX_SIZE)]
	ldp	x27, x28, [x0, #(TLKD_C_RT_CTX_X27 - TLKD_C_RT_CTX_SIZE)]
	ldp	x29, x30, [x0, #(TLKD_C_RT_CTX_X29 - TLKD_C_RT_CTX_SIZE)]

	/* ------------------------------------------------
	 * This should take us back to the instruction
	 * after the call to the last tlkd_enter_sp().
	 * Place the second parameter to x0 so that the
	 * caller will see it as a return value from the
	 * original entry call
	 * ------------------------------------------------
	 */
	mov	x0, x1
	ret
endfunc tlkd_exit_sp
