/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>

	.globl	console_init
	.globl	console_uninit
	.globl	console_putc
	.globl	console_getc
	.globl	console_flush

	/*
	 *  The console base is in the data section and not in .bss
	 *  even though it is zero-init. In particular, this allows
	 *  the console functions to start using this variable before
	 *  the runtime memory is initialized for images which do not
	 *  need to copy the .data section from ROM to RAM.
	 */
.section .data.console_base ; .align 2
	console_base: .word 0x0

	/* -----------------------------------------------
	 * int console_init(uintptr_t base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. It saves
	 * the console base to the data section.
	 * In: r0 - console base address
	 *     r1 - Uart clock in Hz
	 *     r2 - Baud rate
	 * out: return 1 on success else 0 on error
	 * Clobber list : r1 - r3
	 * -----------------------------------------------
	 */
func console_init
	/* Check the input base address */
	cmp	r0, #0
	beq	init_fail
	ldr	r3, =console_base
	str	r0, [r3]
	b	console_core_init
init_fail:
	bx	lr
endfunc console_init

	/* -----------------------------------------------
	 * void console_uninit(void)
	 * Function to finish the use of console driver.
	 * It sets the console_base as NULL so that any
	 * further invocation of `console_putc` or
	 * `console_getc` APIs would return error.
	 * -----------------------------------------------
	 */
func console_uninit
	mov	r0, #0
	ldr	r3, =console_base
	str	r0, [r3]
	bx	lr
endfunc console_uninit

	/* ---------------------------------------------
	 * int console_putc(int c)
	 * Function to output a character over the
	 * console. It returns the character printed on
	 * success or -1 on error.
	 * In : r0 - character to be printed
	 * Out : return -1 on error else return character.
	 * Clobber list : r1, r2
	 * ---------------------------------------------
	 */
func console_putc
	ldr	r2, =console_base
	ldr	r1, [r2]
	b	console_core_putc
endfunc console_putc

	/* ---------------------------------------------
	 * int console_getc(void)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_getc
	ldr	r1, =console_base
	ldr	r0, [r1]
	b	console_core_getc
endfunc console_getc

	/* ---------------------------------------------
	 * int console_flush(void)
	 * Function to force a write of all buffered
	 * data that hasn't been output. It returns 0
	 * upon successful completion, otherwise it
	 * returns -1.
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_flush
	ldr	r1, =console_base
	ldr	r0, [r1]
	b	console_core_flush
endfunc console_flush
