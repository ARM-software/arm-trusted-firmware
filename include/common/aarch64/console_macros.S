/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __CONSOLE_MACROS_S__
#define __CONSOLE_MACROS_S__

#include <asm_macros.S>
#include <console.h>

/*
 * This file defines macros for use by console drivers that want to hook into
 * the generic console API. Each console driver needs to include this file, add
 * a declare_console directive and implement at least func_console_init. All
 * other functions are optional. Functions need to be declared through these
 * macros to provide backwards-compatiblity with the previous console API for
 * older platform code. All functions need to take care to obey the register
 * restrictions outlined below and not change any register not explicitly listed
 * in the clobber list. In addition to the listed registers, implementations are
 * always allowed to use the intra-procedure-call registers (X16 and X17) as
 * temporary scratch registers within a single function.
 */

	/* ------------------------------------------------
	 * int console_<driver>_init(uintptr_t baseaddr,
	 *                           ...args...)
	 * Initializes the console. Will be called directly
	 * from console_<driver>_register and receive any
	 * arguments in x0-x7 unchanged from it. x0 is
	 * intended to be a base address pointer, but can
	 * be used for any purpose. It will be stored in
	 * the console structure and passed in to every
	 * other callback for this console.
	 * In: x0 - driver-specific base address
	 *     x1...x7 - optional extra arguments
	 * Out: x0 - 1 for success, 0 for error
	 * Clobber list: x0 - x7
	 * ------------------------------------------------
	 */
.macro func_console_init _driver
	func console_\_driver\()_init
	.globl console_core_init
	.weak console_core_init
	console_core_init:
.endm

	/* ------------------------------------------------
	 * void console_<driver>_putc(int c, uintptr_t base)
	 * Outputs the character c on the console. Has no
	 * return value and must preserve x0 even on error.
	 * In: x0 - character to output
	 *     x1 - baseaddr passed in on initialization
	 * Clobber list: x1, x2 (NOT x0!!!)
	 * ------------------------------------------------
	 */
.macro func_console_putc _driver
	func console_\_driver\()_putc
	.globl console_core_putc
	.weak console_core_putc
	console_core_putc:
.endm

	/* ------------------------------------------------
	 * int console_<driver>_getc(uintptr_t baseaddr)
	 * Reads a character from the console.
	 * In: x0 - baseaddr passed in on initialization
	 * Out: x0 - read character, or -1 if not available
	 * Clobber list: x0, x1
	 * ------------------------------------------------
	 */
.macro func_console_getc _driver
	func console_\_driver\()_getc
	.globl console_core_getc
	.weak console_core_getc
	console_core_getc:
.endm

	/* ------------------------------------------------
	 * void console_<driver>_flush(uintptr_t baseaddr)
	 * Write out all buffered data. Should only return
	 * after the buffers have been completely flushed
	 * (or on an unrecoverable error condition).
	 * In: x0 - baseaddr passed in on initialization
	 * Clobber list: x0, x1, x2, x3
	 * ------------------------------------------------
	 */
.macro func_console_flush _driver
	func console_\_driver\()_flush
	.globl console_core_flush
	.weak console_core_flush
	console_core_flush:
.endm

.macro declare_console _driver _boot=1 _runtime=0 _crash=1
	.section .data.console_\_driver\()_info
	.align 3
	console_\_driver\()_info:
	.quad 0x0
	.quad (\_boot << CONSOLE_FLAG_BOOT_SHIFT) | \
	      (\_runtime << CONSOLE_FLAG_RUNTIME_SHIFT) | \
	      (\_crash << CONSOLE_FLAG_CRASH_SHIFT)
	.quad 0x0
	.quad console_\_driver\()_putc
	.quad console_\_driver\()_getc
	.quad console_\_driver\()_flush

	/* --------------------------------------------------
	 * int console_<driver>_register(uintptr_t base, ...)
	 * Registers the <driver> console in the list of
	 * active consoles. Calls console_<driver>_init and
	 * passes all arguments in x0-x7 through to it
	 * unchanged. The meaning of the arguments depends
	 * on the implementation of console_<driver>_init.
	 * In: x0...x7 - arguments for console_<driver>_init
	 * Out: x0 - return value from console_<driver>_init
	 * Clobber list: x0 - x7, x14, x15
	 * --------------------------------------------------
	 */
	.globl console_\_driver\()_register
	func console_\_driver\()_register
		mov	x15, x30
		adrp	x14, console_\_driver\()_info
		add	x14, x14, :lo12:console_\_driver\()_info
		str	x0, [x14, #CONSOLE_INFO_BASE]
		bl	console_\_driver\()_init
		cbz	x0, 1f
		mov	x0, x14
		bl	console_register
		mov	x0, #1
	1:	ret	x15
	endfunc console_\_driver\()_register

	/* --------------------------------------------------
	 * struct console_info *console_<driver>_unregister(void)
	 * Removes the <driver> console from the list of
	 * active consoles.
	 * Out: x0 - address of removed console_info struct,
	 *           or NULL if the console wasn't in the list
	 * Clobber list: x0, x1, x14
	 * --------------------------------------------------
	 */
	.globl console_\_driver\()_unregister
	func console_\_driver\()_unregister
		adrp	x0, console_\_driver\()_info
		add	x0, x0, :lo12:console_\_driver\()_info
		b	console_unregister
	endfunc console_\_driver\()_unregister

	/* Mark all callbacks as weak so their implementation is optional. */
	.weak console_\_driver\()_putc
	.weak console_\_driver\()_getc
	.weak console_\_driver\()_flush
	.weak console_\_driver\()_uninit
.endm

#endif /* __CONSOLE_MACROS_S__ */
