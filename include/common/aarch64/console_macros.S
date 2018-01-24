/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __CONSOLE_MACROS_S__
#define __CONSOLE_MACROS_S__

#include <console.h>

/*
 * This macro encapsulates the common setup that has to be done at the end of
 * a console driver's register function. It will register all of the driver's
 * callbacks in the console_t structure and initialize the flags field (by
 * default consoles are enabled for the "boot" and "crash" states, this can be
 * changed after registration with the console_set_scope() function). It ends
 * with a tail call that will include return to the caller.
 * REQUIRES console_t pointer in x0 and a valid return address in x30.
 */
	.macro	finish_console_register _driver
	/*
	 * Add these weak definitions so we will automatically write a 0 if the
	 * function doesn't exist. I'd rather use .ifdef but that only works if
	 * the function was defined (not just declared .global) above this point
	 * in the file, which we can't guarantee.
	 */
	.weak console_\_driver\()_putc
	.weak console_\_driver\()_getc
	.weak console_\_driver\()_flush

	/* Don't use adrp on weak funcs! See GNU ld bugzilla issue 22589. */
	ldr	x1, =console_\_driver\()_putc
	str	x1, [x0, #CONSOLE_T_PUTC]
	ldr	x1, =console_\_driver\()_getc
	str	x1, [x0, #CONSOLE_T_GETC]
	ldr	x1, =console_\_driver\()_flush
	str	x1, [x0, #CONSOLE_T_FLUSH]
	mov	x1, #(CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH)
	str	x1, [x0, #CONSOLE_T_FLAGS]
	b	console_register
	.endm

#endif /* __CONSOLE_MACROS_S__ */
