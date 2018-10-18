/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>

	.globl	console_init
	.globl	console_putc
	.globl	console_uninit
	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc
	.globl	console_flush

	.extern	rcar_log_init
	.extern	rcar_set_log_data

	/* -----------------------------------------------
	 * int console_core_init(unsigned long base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the log area. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * Return 1 on SUCCESS, 0 on error
	 * In: x0 - Not used
	 *     w1 - Not used
	 *     w2 - Not used
	 * -----------------------------------------------
	 */
func console_core_init
	b	rcar_log_init
endfunc console_core_init
func  console_init
	b	console_core_init
endfunc console_init

	/* --------------------------------------------------------
	 * int console_core_putc(int c, unsigned long base_addr)
	 * Function to output a character over the log area.
	 * Return 1 on SUCCESS, 0 on error
	 * In : w0 - Not used
	 *      x1 - Not used
	 * --------------------------------------------------------
	 */
func console_core_putc
	b	rcar_set_log_data
endfunc console_core_putc
func console_putc
	b	console_core_putc
endfunc console_putc

	/* ---------------------------------------------
	 * int console_core_getc(unsigned long base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In : x0 - console base address
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_core_getc
	ret
endfunc console_core_getc

	/* -----------------------------------------------
	 * void console_uninit(void)
	 * Function to finish the use of console driver.
	 * -----------------------------------------------
	 */
func console_uninit
	ret
endfunc console_uninit

	/* ---------------------------------------------
	 * int console_flush(void)
	 * Function to force a write of all buffered
	 * data that hasn't been output. It returns 0
	 * upon successful completion, otherwise it
	 * returns -1.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_flush
	mov	w0, #0
	ret
endfunc console_flush
