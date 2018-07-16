/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>

	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc
	.globl	console_core_flush

	/* -----------------------------------------------------------------
	 * int console_core_init(uintptr_t base_addr,
	 *			 unsigned int uart_clk,
	 *			 unsigned int baud_rate)
	 *
	 * Function to initialize the console without a C Runtime to print
	 * debug information. This function will be accessed by console_init
	 * and crash reporting.
	 *
	 * In: r0 - console base address
	 *     r1 - Uart clock in Hz
	 *     r2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : r1, r2, r3
	 * -----------------------------------------------------------------
	 */
func console_core_init
	bx	lr
endfunc console_core_init

	/* ---------------------------------------------------------------
	 * int console_core_putc(int c, uintptr_t base_addr)
	 *
	 * Function to output a character over the console. It returns the
	 * character printed on success or -1 on error.
	 *
	 * In : r0 - character to be printed
	 *      r1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : r2
	 * ---------------------------------------------------------------
	 */
func console_core_putc
	bx	lr
endfunc console_core_putc

	/* -----------------------------------------------------------
	 * int console_core_getc(uintptr_t base_addr)
	 *
	 * Function to get a character from the console.
	 * It returns the character grabbed on success or -1 on error.
	 *
	 * In : r0 - console base address
	 * Out : return -1.
	 * Clobber list : r0, r1
	 * -----------------------------------------------------------
	 */
func console_core_getc
	/* Not supported */
	mov	r0, #-1
	bx	lr
endfunc console_core_getc

	/* ---------------------------------------------------------------
	 * int console_core_flush(uintptr_t base_addr)
	 *
	 * Function to force a write of all buffered data that hasn't been
	 * output.
	 *
	 * In : r0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : r0, r1
	 * ---------------------------------------------------------------
	 */
func console_core_flush
	bx	lr
endfunc console_core_flush
