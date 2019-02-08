/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <setjmp.h>

	.globl	setjmp
	.globl	longjmp

/*
 * int setjmp(jmp_buf env);
 */
func setjmp
	mov	x7, sp

	stp	x19, x20, [x0, #JMP_CTX_X19]
	stp	x21, x22, [x0, #JMP_CTX_X21]
	stp	x23, x24, [x0, #JMP_CTX_X23]
	stp	x25, x26, [x0, #JMP_CTX_X25]
	stp	x27, x28, [x0, #JMP_CTX_X27]
	stp	x29, x30, [x0, #JMP_CTX_X29]
	stp	x7, xzr, [x0, #JMP_CTX_SP]

	mov	x0, #0
	ret
endfunc setjmp


/*
 * void longjmp(jmp_buf env, int val);
 */
func longjmp
	ldp	x7, xzr, [x0, #JMP_CTX_SP]

#if ENABLE_ASSERTIONS
	/*
	 * Since we're unwinding the stack, assert that the stack being reset to
	 * is shallower.
	 */
	mov	x19, sp
	cmp	x7, x19
	ASM_ASSERT(ge)
#endif

	ldp	x19, x20, [x0, #JMP_CTX_X19]
	ldp	x21, x22, [x0, #JMP_CTX_X21]
	ldp	x23, x24, [x0, #JMP_CTX_X23]
	ldp	x25, x26, [x0, #JMP_CTX_X25]
	ldp	x27, x28, [x0, #JMP_CTX_X27]
	ldp	x29, x30, [x0, #JMP_CTX_X29]

	mov	sp, x7

	ands	x0, x1, x1 /* Move val to x0 and set flags */
	cinc	x0, x0, eq /* If val is 0, return 1 */
	ret
endfunc longjmp
