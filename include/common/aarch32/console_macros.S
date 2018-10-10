/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
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
/*
 * The USE_FINISH_CONSOLE_REG_2 guard is introduced to allow selection between
 * the 2 variants of the finish_console_register macro and will be removed
 * once the deprecated variant is removed.
 */
#ifndef USE_FINISH_CONSOLE_REG_2
#if !ERROR_DEPRECATED
	/* This version of the macro is deprecated. Use the new version */
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
	ldr	r1, =console_\_driver\()_putc
	str	r1, [r0, #CONSOLE_T_PUTC]
	ldr	r1, =console_\_driver\()_getc
	str	r1, [r0, #CONSOLE_T_GETC]
	ldr	r1, =console_\_driver\()_flush
	str	r1, [r0, #CONSOLE_T_FLUSH]
	mov	r1, #(CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH)
	str	r1, [r0, #CONSOLE_T_FLAGS]
	b	console_register
	.endm
#endif /* ERROR_DEPRECATED */
#else /* USE_FINISH_CONSOLE_REG_2 */
	/* The new version of the macro not using weak references */
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

	.ifne \getc
	  ldr	r1, =console_\_driver\()_getc
	.else
	  mov	r1, #0
	.endif
	str	r1, [r0, #CONSOLE_T_GETC]

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
#endif /* USE_FINISH_CONSOLE_REG_2 */
#endif /* __CONSOLE_MACROS_S__ */
