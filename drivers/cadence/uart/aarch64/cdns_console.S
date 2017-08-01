/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <console_macros.S>
#include <cadence/cdns_uart.h>

declare_console cadence

	/* -----------------------------------------------
	 * int console_cadence_init(unsigned long base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * We assume that the bootloader already set up
	 * the HW (baud, ...) and only enable the trans-
	 * mitter and receiver here.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x1, x2, x3
	 * -----------------------------------------------
	 */
func_console_init cadence
	/* Check the input base address */
	cbz	x0, core_init_fail
	/* Check baud rate and uart clock for sanity */
	cbz	w1, core_init_fail
	cbz	w2, core_init_fail

	/* RX/TX enabled & reset */
	mov	w3, #(R_UART_CR_TX_EN | R_UART_CR_RX_EN | R_UART_CR_TXRST | R_UART_CR_RXRST)
	str	w3, [x0, #R_UART_CR]

	mov	w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_cadence_init

	/* --------------------------------------------------------
	 * void console_cadence_putc(int c, unsigned long base_addr)
	 * Function to output a character over the console. Must
	 * not modify the value in x0.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func_console_putc cadence
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
	ret
endfunc console_cadence_putc

	/* ---------------------------------------------
	 * int console_cadence_getc(unsigned long base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : x0 - console base address
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func_console_getc cadence
	cbz	x0, getc_no_char
	/* Check if the receive FIFO is empty */
	ldr	w1, [x0, #R_UART_SR]
	tbnz	w1, #UART_SR_INTR_REMPTY_BIT, getc_no_char
	ldr	w1, [x0, #R_UART_RX]
	mov	w0, w1
	ret
getc_no_char:
	mov	w0, #-1
	ret
endfunc console_cadence_getc

	/* ---------------------------------------------
	 * void console_cadence_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func_console_flush cadence
	/* Placeholder */
	mov	w0, #0
	ret
endfunc console_cadence_flush
