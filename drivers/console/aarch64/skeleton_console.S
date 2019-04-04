/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>
#include <console_macros.S>

	/*
	 * This file contains a skeleton console driver that can be used as
	 * basis for a real console driver. Console drivers in Trusted Firmware
	 * can be instantiated multiple times. Each instance is described by a
	 * separate console_t structure which must be registered with the common
	 * console framework via console_register(). Console drivers should
	 * define a console_xxx_register() function that initializes a new
	 * console_t structure passed in from the caller and registers it after
	 * initializing the console hardware. Drivers may define their own
	 * structures extending console_t to store private driver information.
	 * Console drivers *MUST* take care that the console callbacks they
	 * implement only change registers allowed in the clobber lists defined
	 * in this file. (Note that in addition to the explicit clobber lists,
	 * any function may always clobber the intra-procedure-call registers
	 * X16 and X17, but may never depend on them retaining their values
	 * across any function call.)
	 * Platforms using drivers based on this template need to enable
	 * MULTI_CONSOLE_API := 1 in their platform.mk.
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
	 * In : x0 - pointer to empty console_t structure
	 *      x1 through x7: additional parameters as desired
	 * Out: x0 - 1 on success, 0 on error
	 * Clobber list : x0 - x7
	 * -----------------------------------------------
	 */
func console_xxx_register
	/*
	 * Store parameters (e.g. hardware base address) in driver-specific
	 * console_xxx_t structure field if they will need to be retrieved
	 * by later console callback (e.g. putc).
	 * Example:
	 */
	str	x1, [x0, #CONSOLE_T_XXX_BASE]
	str	x2, [x0, #CONSOLE_T_XXX_SOME_OTHER_VALUE]

	/*
	 * Initialize console hardware, using x1 - x7 parameters as needed.
	 * Keep console_t pointer in x0 for later.
	 */

	/*
	 * Macro to finish up registration and return (needs valid x0 + x30).
	 * If any of the argument is unspecified, then the corresponding
	 * entry in console_t is set to 0.
	 */
	finish_console_register xxx putc=1, getc=1, flush=1

	/* Jump here if hardware init fails or parameters are invalid. */
register_fail:
	mov	w0, #0
	ret
endfunc console_xxx_register

	/* --------------------------------------------------------
	 * int console_xxx_putc(int c, console_xxx_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t struct
	 * Out: w0 - printed character on success, < 0 on error.
	 * Clobber list : x0, x1, x2
	 * --------------------------------------------------------
	 */
func console_xxx_putc
	/*
	 * Retrieve values we need (e.g. hardware base address) from
	 * console_xxx_t structure pointed to by x1.
	 * Example:
	 */
	ldr	x1, [x1, #CONSOLE_T_XXX_BASE]

	/*
	 * Write w0 to hardware.
	 */

	ret

	/* Jump here if output fails for any reason. */
putc_error:
	mov	w0, #-1
	ret
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
	 * In : x0 - pointer to console_t struct
	 * Out: w0 - character on success,
	 *           ERROR_NO_PENDING_CHAR if no char,
	 *           < -1 on error
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_xxx_getc
	/*
	 * Retrieve values we need (e.g. hardware base address) from
	 * console_xxx_t structure pointed to by x0.
	 * Example:
	 */
	ldr	x1, [x0, #CONSOLE_T_XXX_BASE]

	/*
	 * Try to read character into w0 from hardware.
	 */

	ret

	/* Jump here if there is no character available at this time. */
getc_no_char:
	mov	w0, #ERROR_NO_PENDING_CHAR
	ret

	/* Jump here if there was any hardware error. */
getc_error:
	mov	w0, #-2		/* may pick error codes between -2 and -127 */
	ret
endfunc console_xxx_getc

	/* ---------------------------------------------
	 * int console_xxx_flush(console_xxx_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - pointer to console_xxx_t struct
	 * Out: w0 - 0 on success, < 0 on error
	 * Clobber list : x0, x1, x2, x3, x4, x5
	 * ---------------------------------------------
	 */
func console_xxx_flush
	/*
	 * Retrieve values we need (e.g. hardware base address) from
	 * console_xxx_t structure pointed to by x0.
	 * Example:
	 */
	ldr	x1, [x0, #CONSOLE_T_XXX_BASE]

	/*
	 * Flush all remaining output from hardware FIFOs. Do not return until
	 * all data has been flushed or there was an unrecoverable error.
	 */

	mov	w0, #0
	ret

	/* Jump here if an unrecoverable error has been encountered. */
flush_error:
	mov	w0, #-1
	ret
endfunc console_xxx_flush
