/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>
#include <console_macros.S>

	/*
	 * This file contains a skeleton console driver that can be used as a
	 * basis for a real console driver. Console drivers in Trusted Firmware
	 * can be instantiated multiple times. Each instance is described by a
	 * separate console_t structure which must be registered with the common
	 * console framework via console_register(). Console drivers should
	 * define a console_xxx_register() function that initializes a new
	 * console_t structure passed in from the caller and registers it after
	 * initializing the console hardware. Drivers may define their own
	 * structures extending console_t to store private driver information.
	 * Console drivers *MUST* ensure that the console callbacks they
	 * implement only change registers allowed in the clobber lists defined
	 * in this file. (Note that in addition to the explicit clobber lists,
	 * any function may always clobber the intra-procedure-call register
	 * r12, but may never depend on it retaining its value across any
	 * function call.)
	 */

	.globl	console_xxx_register
	.globl	console_xxx_putc
	.globl	console_xxx_getc
	.globl	console_xxx_flush

	/* -----------------------------------------------
	 * int console_xxx_register(console_xxx_t *console,
	 * 	...additional parameters as desired...)
	 * Function to initialize and register the console.
	 * The caller needs to pass an empty console_xxx_t
	 * structure in which *MUST* be allocated in
	 * persistent memory (e.g. a global or static local
	 * variable, *NOT* on the stack).
	 * In : r0 - pointer to empty console_t structure
	 *      r1 through r7: additional parameters as desired
	 * Out: r0 - 1 on success, 0 on error
	 * Clobber list : r0 - r7
	 * -----------------------------------------------
	 */
func console_xxx_register
	/*
	 * Store parameters (e.g. hardware base address) in driver-specific
	 * console_xxx_t structure field if they will need to be retrieved
	 * by later console callback (e.g. putc).
	 * Example:
	 */
	str	r1, [r0, #CONSOLE_T_XXX_BASE]
	str	r2, [r0, #CONSOLE_T_XXX_SOME_OTHER_VALUE]

	/*
	 * Initialize console hardware, using r1 - r7 parameters as needed.
	 * Keep console_t pointer in r0 for later.
	 */

	/*
	 * Macro to finish up registration and return (needs valid r0 + lr).
	 * If any of the argument is unspecified, then the corresponding
	 * entry in console_t is set to 0.
	 */
	finish_console_register xxx putc=1, getc=1, flush=1

	/* Jump here if hardware init fails or parameters are invalid. */
register_fail:
	mov	r0, #0
	bx	lr
endfunc console_xxx_register

	/* --------------------------------------------------------
	 * int console_xxx_putc(int c, console_xxx_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : r0 - character to be printed
	 *      r1 - pointer to console_t struct
	 * Out: r0 - printed character on success, < 0 on error.
	 * Clobber list : r0, r1, r2
	 * --------------------------------------------------------
	 */
func console_xxx_putc
	/*
	 * Retrieve values we need (e.g. hardware base address) from
	 * console_xxx_t structure pointed to by r1.
	 * Example:
	 */
	ldr	r1, [r1, #CONSOLE_T_XXX_BASE]

	/*
	 * Write r0 to hardware.
	 */

	bx	lr

	/* Jump here if output fails for any reason. */
putc_error:
	mov	r0, #-1
	bx	lr
endfunc console_xxx_putc

	/* ---------------------------------------------
	 * int console_xxx_getc(console_xxx_t *console)
	 * Function to get a character from the console.
	 * Even though console_getc() is blocking, this
	 * callback has to be non-blocking and always
	 * return immediately to allow polling multiple
	 * drivers concurrently.
	 * Returns the character grabbed on success,
	 * ERROR_NO_PENDING_CHAR if no character was
	 * available at this time, or any value
	 * between -2 and -127 if there was an error.
	 * In : r0 - pointer to console_t struct
	 * Out: r0 - character on success,
	 *           ERROR_NO_PENDING_CHAR if no char,
	 *           < -1 on error
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_xxx_getc
	/*
	 * Retrieve values we need (e.g. hardware base address) from
	 * console_xxx_t structure pointed to by r0.
	 * Example:
	 */
	ldr	r1, [r0, #CONSOLE_T_XXX_BASE]

	/*
	 * Try to read character into r0 from hardware.
	 */

	bx	lr

	/* Jump here if there is no character available at this time. */
getc_no_char:
	mov	r0, #ERROR_NO_PENDING_CHAR
	bx	lr

	/* Jump here if there was any hardware error. */
getc_error:
	mov	r0, #-2		/* may pick error codes between -2 and -127 */
	bx	lr
endfunc console_xxx_getc

	/* ---------------------------------------------
	 * int console_xxx_flush(console_xxx_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : r0 - pointer to console_xxx_t struct
	 * Out: r0 - 0 on success, < 0 on error
	 * Clobber list : r0, r1, r2, r3, r4, r5
	 * ---------------------------------------------
	 */
func console_xxx_flush
	/*
	 * Retrieve values we need (e.g. hardware base address) from
	 * console_xxx_t structure pointed to by r0.
	 * Example:
	 */
	ldr	r1, [r0, #CONSOLE_T_XXX_BASE]

	/*
	 * Flush all remaining output from hardware FIFOs. Do not return until
	 * all data has been flushed or there was an unrecoverable error.
	 */

	mov	r0, #0
	bx	lr

	/* Jump here if an unrecoverable error has been encountered. */
flush_error:
	mov	r0, #-1
	bx	lr
endfunc console_xxx_flush
