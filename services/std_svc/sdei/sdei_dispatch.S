/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	begin_sdei_synchronous_dispatch

/*
 * void begin_sdei_synchronous_dispatch(jmp_buf *buffer);
 *
 * Begin SDEI dispatch synchronously by setting up a jump point, and exiting
 * EL3. This jump point is jumped to by the dispatcher after the event is
 * completed by the client.
 */
func begin_sdei_synchronous_dispatch
	stp	x30, xzr, [sp, #-16]!
	bl	setjmp
	cbz	x0, 1f
	ldp	x30, xzr, [sp], #16
	ret
1:
	b	el3_exit
endfunc begin_sdei_synchronous_dispatch
