/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <cadence/cdns_uart.h>

	.globl  console_cdns_init
	.globl  console_cdns_putc
	.globl  console_cdns_getc
	.globl	console_cdns_flush

#if MULTI_CONSOLE_API
	.globl console_cdns_register

	/* -----------------------------------------------
	 * int console_cdns_register(console_cdns_t *console,
		uintptr_t base, uint32_t clk, uint32_t baud)
	 * Function to initialize and register a new CDNS
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: x0 - pointer to empty console_cdns_t struct
	 *     x1 - UART register base address
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -----------------------------------------------
	 */
func console_cdns_register
	mov	x7, x30
	cbz	x0, register_fail
	str	x1, [x0, #CONSOLE_T_CDNS_BASE]

	/* shift registers around to match ABI of legacy init function */
	mov	x6, x0
	mov	x0, x1

	bl	console_cdns_init
	cbz	x0, register_fail

	mov	x0, x6
	adrp	x1, console_cdns_putc
	add	x1, x1, :lo12:console_cdns_putc
	str	x1, [x0, #CONSOLE_T_PUTC]
	adrp	x1, console_cdns_getc
	add	x1, x1, :lo12:console_cdns_getc
	str	x1, [x0, #CONSOLE_T_GETC]
	mov	x1, #0
	str	x1, [x0, #CONSOLE_T_FLUSH]
	mov	x1, #(1 << CONSOLE_FLAG_BOOT_SHIFT | \
		      1 << CONSOLE_FLAG_CRASH_SHIFT)
	str	x1, [x0, #CONSOLE_T_FLAGS]
	bl	console_register
register_fail:
	ret	x7
endfunc console_cdns_register
#else
	.globl console_core_init
	.globl console_core_putc
	.globl console_core_getc
	.globl console_core_flush
	.equ console_core_init,console_cdns_init
	.equ console_core_putc,console_cdns_putc
	.equ console_core_getc,console_cdns_getc
#endif

	/* -----------------------------------------------
	 * int console_cdns_init(unsigned long base_addr)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * We assume that the bootloader already set up
	 * the HW (baud, ...) and only enable the trans-
	 * mitter and receiver here.
	 * In: x0 - console base address
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x1, x2, x3
	 * -----------------------------------------------
	 */
func console_cdns_init
	/* Check the input base address */
	cbz	x0, core_init_fail

	/* RX/TX enabled & reset */
	mov	w3, #(R_UART_CR_TX_EN | R_UART_CR_RX_EN | R_UART_CR_TXRST | R_UART_CR_RXRST)
	str	w3, [x0, #R_UART_CR]

	mov	w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_cdns_init

	/* --------------------------------------------------------
	 * int console_core_putc(int c, console_cdns_t *cdns)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t struct
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_cdns_putc
#if MULTI_CONSOLE_API
	ldr	x1, [x1, #CONSOLE_T_CDNS_BASE]
#endif
	/* Check the input parameter */
	cbz	x1, putc_error
	/* Prepend '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
1:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #R_UART_SR]
	tbnz	w2, #UART_SR_INTR_TFUL_BIT, 1b
	mov	w2, #0xD
	str	w2, [x1, #R_UART_TX]
2:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #R_UART_SR]
	tbnz	w2, #UART_SR_INTR_TFUL_BIT, 2b
	str	w0, [x1, #R_UART_TX]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_cdns_putc

	/* ---------------------------------------------
	 * int console_cdns_getc(console_cdns_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : x0 - pointer to console_t struct
	 * Out: w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_cdns_getc
#if MULTI_CONSOLE_API
	ldr	x0, [x0, #CONSOLE_T_CDNS_BASE]
#endif
	/* Check if the receive FIFO is empty */
	ldr	w1, [x0, #R_UART_SR]
	tbnz	w1, #UART_SR_INTR_REMPTY_BIT, no_char
	ldr	w1, [x0, #R_UART_RX]
	mov	w0, w1
	ret
no_char:
	mov	w0, #-1
	ret
endfunc console_cdns_getc

	/* ---------------------------------------------
	 * int console_core_flush(uintptr_t base_addr)
	 * DEPRECATED: Not used with MULTI_CONSOLE_API!
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_core_flush
	/* Placeholder */
	mov	w0, #0
	ret
endfunc console_core_flush
