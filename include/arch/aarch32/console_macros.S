/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CONSOLE_MACROS_S
#define CONSOLE_MACROS_S

#include <drivers/console.h>

/*
 * This macro encapsulates the common setup that has to be done at the end of
 * a console driver's register function. It will register all of the driver's
 * callbacks in the console_t structure and initialize the flags field (by
 * default consoles are enabled for the "boot" and "crash" states, this can be
 * changed after registration with the console_set_scope() function). It ends
 * with a tail call that will include return to the caller.
 * REQUIRES console_t pointer in r0 and a valid return address in lr.
 */
	.macro	finish_console_register _driver, putc=0, getc=0, flush=0
	/*
	 * If any of the callback is not specified or set as 0, then the
	 * corresponding callback entry in console_t is set to 0.
	 */
	.ifne \putc
	  ldr	r1, =console_\_driver\()_putc
	.else
	  mov	r1, #0
	.endif
	str	r1, [r0, #CONSOLE_T_PUTC]

	/*
	 * If ENABLE_CONSOLE_GETC support is disabled, but a getc callback is
	 * specified nonetheless, the assembler will abort on encountering the
	 * CONSOLE_T_GETC macro, which is undefined.
	 */
	.ifne \getc
	  ldr	r1, =console_\_driver\()_getc
	  str	r1, [r0, #CONSOLE_T_GETC]
	.else
#if ENABLE_CONSOLE_GETC
	  mov	r1, #0
	  str	r1, [r0, #CONSOLE_T_GETC]
#endif
	.endif

	.ifne \flush
	  ldr	r1, =console_\_driver\()_flush
	.else
	  mov	r1, #0
	.endif
	str	r1, [r0, #CONSOLE_T_FLUSH]

	mov	r1, #(CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH)
	str	r1, [r0, #CONSOLE_T_FLAGS]
	b	console_register
	.endm

#endif /* CONSOLE_MACROS_S */
