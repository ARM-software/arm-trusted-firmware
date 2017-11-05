/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

/*
 * EABI wrappers from the udivmod and idivmod functions
 */

	.globl ret_uidivmod_values
	.globl ret_idivmod_values

/*
 * signed ret_idivmod_values(signed quot, signed rem);
 * return quotient and remaining the EABI way (regs r0,r1)
 */
func ret_idivmod_values
        bx lr
endfunc ret_idivmod_values

/*
 * unsigned ret_uidivmod_values(unsigned quot, unsigned rem);
 * return quotient and remaining the EABI way (regs r0,r1)
 */
func ret_uidivmod_values
        bx      lr
endfunc ret_uidivmod_values
