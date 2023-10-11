/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
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
 * REQUIRES console_t pointer in x0 and a valid return address in x30.
 */
	.macro	finish_console_register _driver, putc=0, getc=0, flush=0
	/*
	 * If any of the callback is not specified or set as 0, then the
	 * corresponding callback entry in console_t is set to 0.
	 */
	.ifne \putc
	  adrp	x1, console_\_driver\()_putc
	  add	x1, x1, :lo12:console_\_driver\()_putc
	  str	x1, [x0, #CONSOLE_T_PUTC]
	.else
	  str	xzr, [x0, #CONSOLE_T_PUTC]
	.endif

	/*
	 * If ENABLE_CONSOLE_GETC support is disabled, but a getc callback is
	 * specified nonetheless, the assembler will abort on encountering the
	 * CONSOLE_T_GETC macro, which is undefined.
	 */
	.ifne \getc
	  adrp	x1, console_\_driver\()_getc
	  add	x1, x1, :lo12:console_\_driver\()_getc
	  str	x1, [x0, #CONSOLE_T_GETC]
	.else
#if ENABLE_CONSOLE_GETC
	  str	xzr, [x0, #CONSOLE_T_GETC]
#endif
	.endif

	.ifne \flush
	  adrp	x1, console_\_driver\()_flush
	  add	x1, x1, :lo12:console_\_driver\()_flush
	  str	x1, [x0, #CONSOLE_T_FLUSH]
	.else
	  str	xzr, [x0, #CONSOLE_T_FLUSH]
	.endif

	mov	x1, #(CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH)
	str	x1, [x0, #CONSOLE_T_FLAGS]
	b	console_register
	.endm

#endif /* CONSOLE_MACROS_S */
