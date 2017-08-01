/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <console_macros.S>

	/*
	 * This file contains a skeleton console implementation that can
	 * be used as basis for a real console implementation by platforms
	 * that do not contain PL011 hardware. Find and replace 'skeleton'
	 * with your driver name. You only need to implement init(), all
	 * other functions are optional and can be deleted.
	 * See include/common/aarch64/console_macros.S for details.
	 */

declare_console skeleton

	/* -----------------------------------------------
	 * int console_skeleton_init(uintptr_t base_addr,
	 * ...additional arguments...)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: x0 - console base address
	 *     x1...x7 - optional console-specific args
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x0 - x7
	 * -----------------------------------------------
	 */
func_console_init skeleton
	/* Check the input base address */
	cbz	x0, core_init_fail

	/* Insert implementation here */

	mov	w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_skeleton_init

	/* --------------------------------------------------------
	 * int console_skeleton_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It must
	 * preserve the value in x0 unchanged.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Clobber list : x1, x2 (NOT x0!!!)
	 * --------------------------------------------------------
	 */
func_console_putc skeleton
	/* Check the input parameter */
	cbz	x1, putc_error

	/* Insert implementation here */

putc_error:
	ret
endfunc console_skeleton_putc

	/* ---------------------------------------------
	 * int console_skeleton_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character available.
	 * In : x0 - console base address
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func_console_getc skeleton
	cbz	x0, getc_no_char

	/* Insert implementation here */

	ret
getc_no_char:
	mov	w0, #-1
	ret
endfunc console_skeleton_getc

	/* ---------------------------------------------
	 * void console_skeleton_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Clobber list : x0 - x3
	 * ---------------------------------------------
	 */
func_console_flush skeleton
	cbz	x0, flush_error

	/* Insert implementation here */

flush_error:
	ret
endfunc console_skeleton_flush
