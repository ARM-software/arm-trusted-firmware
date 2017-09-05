/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <console_macros.S>

	.globl	console_register
	.globl	console_unregister
	.globl	console_uninit
	.globl	console_start_crash
	.globl	console_putc
	.globl	console_getc
	.globl	console_flush

	/*
	 *  The console list pointer is in the data section and not in
	 *  .bss even though it is zero-init. In particular, this allows
	 *  the console functions to start using this variable before
	 *  the runtime memory is initialized for images which do not
	 *  need to copy the .data section from ROM to RAM.
	 */
.section .data.console_list ; .align 3
	console_list: .quad 0x0
.section .data.console_state ; .align 0
	console_state: .byte (1 << CONSOLE_FLAG_BOOT_SHIFT)

	/* -----------------------------------------------
	 * void console_register(console_info_t *console)
	 * Function to insert a new console structure into
	 * the console list. Should usually only be called
	 * by declare_console expansions.
	 * In : x0 - address of console structure
	 * Clobber list: x0, x1, x14
	 * -----------------------------------------------
	 */
func console_register
	adrp	x14, console_list
	ldr	x1, [x14, :lo12:console_list]	/* X1 = first struct in list */
	str	x0, [x14, :lo12:console_list]	/* list head = new console */
	str	x1, [x0]			/* new console next ptr = X1 */
	ret
endfunc console_register

#if !ERROR_DEPRECATED
	/* Set up compatiblity symbols for platforms using old console API. */
	declare_console core

	.globl	console_init
	/* -----------------------------------------------
	 * int console_init(uintptr_t base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Deprecated legacy function for initializing a
	 * single console. Current platforms should call
	 * console_<driver>_register for the relevant
	 * driver instead. This needs to clear the console
	 * list first because some legacy platforms called
	 * console_init more than once and expect it to
	 * overwrite previous console state.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Clobber list : x0 - x4, x14, x15
	 * -----------------------------------------------
	 */
func console_init
	adrp	x14, console_list
	str	xzr, [x14, :lo12:console_list]
	b	console_core_register
endfunc console_init
#endif

	/* -----------------------------------------------
	 * console_info_t *console_unregister(
	 *                        console_info_t *console)
	 * Function to find a specific console in the list
	 * of currently active consoles and remove it.
	 * Should usually only be called by
	 * declare_console expansions.
	 * In: x0 - address of console structure to remove
	 * Out: x0 - removed address, or NULL if not found
	 * Clobber list: x0, x1, x14
	 * -----------------------------------------------
	 */
func console_unregister
	adrp	x14, console_list
	add	x14, x14, :lo12:console_list	/* X14 = ptr to first struct */
	ldr	x1, [x14]			/* X3 = first struct */

unregister_loop:
	cbz	x1, unregister_not_found
	cmp	x0, x1
	b.eq	unregister_found
	ldr	x14, [x14]			/* X14 = next ptr of struct */
	ldr	x1, [x14]			/* X1 = next struct */
	b	unregister_loop

unregister_found:
	ldr	x1, [x1]			/* X1 = next struct */
	str	x1, [x14]			/* prev->next = cur->next */
	ret

unregister_not_found:
	mov	x0, #0				/* return NULL if not found */
	ret
endfunc console_unregister

	/* -----------------------------------------------
	 * void console_uninit(void)
	 * Function to switch from boot to runtime state.
	 * The name is misleading for compatibility with
	 * the legacy API -- this used to uninitialize all
	 * consoles, but now it just quiesces consoles
	 * that are not marked as "runtime".
	 * Clobber list: x0, x1
	 * -----------------------------------------------
	 */
func console_uninit
	mov	w0, #(1 << CONSOLE_FLAG_RUNTIME_SHIFT)
	adrp	x1, console_state
	strb	w0, [x1, :lo12:console_state]
	ret
endfunc console_uninit

	/* -----------------------------------------------
	 * int console_start_crash(void)
	 * Function to switch to crash state. Returns 1 in
	 * x0 for convenience with plat_crash_console_init
	 * Out: x0 - always 1
	 * Clobber list: x0, x1
	 * -----------------------------------------------
	 */
func console_start_crash
	mov	w0, #(1 << CONSOLE_FLAG_CRASH_SHIFT)
	adrp	x1, console_state
	strb	w0, [x1, :lo12:console_state]
	mov	x0, #1
	ret
endfunc console_start_crash

	/* ---------------------------------------------
	 * void console_putc(int c)
	 * Function to output a character. Calls all
	 * console's putc() handlers in succession.
	 * In : x0 - character to be printed
	 * Clobber list : x1, x2, x14, x15
	 * ---------------------------------------------
	 */
func console_putc
	mov	x15, x30
	adrp	x14, console_list
	ldr	x14, [x14, :lo12:console_list]	/* X14 = first console struct */

putc_loop:
	cbz	x14, putc_done
	adrp	x1, console_state
	ldrb	w1, [x1, :lo12:console_state]
	ldr	x2, [x14, #CONSOLE_INFO_FLAGS]
	tst	w1, w2
	b.eq	putc_continue
	ldr	x1, [x14, #CONSOLE_INFO_BASE]
	ldr	x2, [x14, #CONSOLE_INFO_PUTC]
	cbz	x2, putc_continue
	blr	x2
putc_continue:
	ldr	x14, [x14]			/* X14 = next struct */
	b	putc_loop

putc_done:
	ret	x15
endfunc console_putc

	/* ---------------------------------------------
	 * int console_getc(void)
	 * Function to get a character from any console.
	 * Keeps looping through all consoles' getc()
	 * handlers until one of them returns a
	 * character, then stops iterating and returns
	 * that character to the caller.
	 * Out : x0 - read character, or -1 if none
	 * Clobber list : x0, x1, x14, x15
	 * ---------------------------------------------
	 */
func console_getc
	mov	x15, x30
getc_try_again:
	adrp	x14, console_list
	ldr	x14, [x14, :lo12:console_list]	/* X14 = first console struct */

getc_loop:
	cbz	x14, getc_try_again		/* if list ended, start again */
	adrp	x0, console_state
	ldrb	w0, [x0, :lo12:console_state]
	ldr	x1, [x14, #CONSOLE_INFO_FLAGS]
	tst	w0, w1
	b.eq	getc_continue
	ldr	x0, [x14, #CONSOLE_INFO_BASE]
	ldr	x1, [x14, #CONSOLE_INFO_GETC]
	cbz	x1, getc_continue
	blr	x1
	cmp	x0, #0				/* if X0 >= 0: return */
	b.ge	getc_done
getc_continue:
	ldr	x14, [x14]			/* X14 = next struct */
	b	getc_loop

getc_found:
	ret	x15
endfunc console_getc

	/* ---------------------------------------------
	 * void console_flush(void)
	 * Function to force a write of all buffered
	 * data that hasn't been output. Calls all
	 * console's flush() handlers in succession.
	 * Clobber list : x0, x1, x2, x3, x14, x15
	 * ---------------------------------------------
	 */
func console_flush
	mov	x15, x30
	adrp	x14, console_list
	ldr	x14, [x14, :lo12:console_list]	/* X14 = first console struct */

flush_loop:
	cbz	x14, flush_done
	adrp	x1, console_state
	ldrb	w1, [x1, :lo12:console_state]
	ldr	x2, [x14, #CONSOLE_INFO_FLAGS]
	tst	w1, w2
	b.eq	flush_continue
	ldr	x1, [x14, #CONSOLE_INFO_BASE]
	ldr	x2, [x14, #CONSOLE_INFO_FLUSH]
	cbz	x2, flush_continue
	blr	x2
flush_continue:
	ldr	x14, [x14]			/* X14 = next struct */
	b	flush_loop

flush_done:
	ret	x15
endfunc console_flush
