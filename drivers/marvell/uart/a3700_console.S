/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <asm_macros.S>
#include <a3700_console.h>

	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc
	.globl	console_core_flush

	/* -----------------------------------------------
	 * int console_core_init(unsigned long base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success
	 * Clobber list : x1, x2, x3
	 * -----------------------------------------------
	 */
func console_core_init
	/* Check the input base address */
	cbz	x0, init_fail
	/* Check baud rate and uart clock for sanity */
	cbz	w1, init_fail
	cbz	w2, init_fail

	/* Program the baudrate */
	/* Divisor =  Uart clock / (16 * baudrate) */
	lsl	w2, w2, #4
	udiv	w2, w1, w2
	and	w2, w2, #0x3ff

	ldr	w3, [x0, #UART_BAUD_REG]
	bic	w3, w3, 0x3ff
	orr	w3, w3, w2
	str	w3, [x0, #UART_BAUD_REG]/* set baud rate divisor */

	/* Set UART to default 16X scheme */
	mov	w3, #0
	str	w3, [x0, #UART_POSSR_REG]

	/*
	 * Wait for the TX FIFO to be empty. If wait for 20ms, the TX FIFO is
	 * still not empty, TX FIFO will reset by all means.
	 */
	mov	w1, #20				/* max time out 20ms */
2:
	/* Check whether TX FIFO is empty */
	ldr	w3, [x0, #UART_STATUS_REG]
	and	w3, w3, #UARTLSR_TXFIFOEMPTY
	cmp	w3, #0
	b.ne	4f

	/* Delay */
	mov	w2, #30000
3:
	sub     w2, w2, #1
	cmp	w2, #0
	b.ne	3b

	/* Check whether 10ms is waited */
	sub     w1, w1, #1
	cmp	w1, #0
	b.ne	2b

4:
	/* Reset FIFO */
	mov	w3, #UART_CTRL_RXFIFO_RESET
	orr	w3, w3, #UART_CTRL_TXFIFO_RESET
	str	w3, [x0, #UART_CTRL_REG]

	/* Delay */
	mov	w2, #2000
1:
	sub     w2, w2, #1
	cmp	w2, #0
	b.ne	1b

	/* No Parity, 1 Stop */
	mov	w3, #0
	str	w3, [x0, #UART_CTRL_REG]

	mov	w0, #1
	ret
init_fail:
	mov	w0, #0
	ret
endfunc console_core_init

	/* --------------------------------------------------------
	 * int console_core_putc(int c, unsigned int base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_core_putc
	/* Check the input parameter */
	cbz	x1, putc_error

	/* Prepend '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
	/* Check if the transmit FIFO is full */
1:	ldr	w2, [x1, #UART_STATUS_REG]
	and	w2, w2, #UARTLSR_TXFIFOFULL
	cmp	w2, #UARTLSR_TXFIFOFULL
	b.eq	1b
	mov	w2, #0xD		/* '\r' */
	str	w2, [x1, #UART_TX_REG]

	/* Check if the transmit FIFO is full */
2:	ldr	w2, [x1, #UART_STATUS_REG]
	and	w2, w2, #UARTLSR_TXFIFOFULL
	cmp	w2, #UARTLSR_TXFIFOFULL
	b.eq	2b
	str	w0, [x1, #UART_TX_REG]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_core_putc

	/* ---------------------------------------------
	 * int console_core_getc(void)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In : w0 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_core_getc
	/* Check if the receive FIFO is empty */
	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_core_getc

	/* ---------------------------------------------
	 * int console_core_flush(uintptr_t base_addr)
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
