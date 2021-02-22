/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <arch.h>
#include <asm_macros.S>
#include <console_macros.S>
#include <drivers/marvell/uart/a3700_console.h>

	/*
	 * "core" functions are low-level implementations that don't require
	 * writable memory and are thus safe to call in BL1 crash context.
	 */
	.globl console_a3700_core_putc
	.globl console_a3700_core_init
	.globl console_a3700_core_getc
	.globl console_a3700_core_flush

	.globl console_a3700_putc
	.globl console_a3700_getc
	.globl console_a3700_flush

	/* -----------------------------------------------
	 * int console_a3700_core_init(unsigned long base_addr,
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
func console_a3700_core_init
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
	 * Wait for the TX (THR and TSR) to be empty. If wait for 3ms, the TX FIFO is
	 * still not empty, TX FIFO will reset by all means.
	 */
	mov	w1, #30				/* max time out 30 * 100 us */
2:
	/* Check whether TX (THR and TSR) is empty */
	ldr	w3, [x0, #UART_STATUS_REG]
	and	w3, w3, #UARTLSR_TXEMPTY
	cmp	w3, #0
	b.ne	4f

	/* Delay */
	mov	w2, #60000	/* 60000 cycles of below 3 instructions on 1200 MHz CPU ~~ 100 us */
3:
	sub     w2, w2, #1
	cmp	w2, #0
	b.ne	3b

	/* Check whether wait timeout expired */
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
endfunc console_a3700_core_init

	.globl console_a3700_register

	/* -----------------------------------------------
	 * int console_a3700_register(console_t *console,
		uintptr_t base, uint32_t clk, uint32_t baud)
	 * Function to initialize and register a new a3700
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: x0 - UART register base address
	 *     w1 - UART clock in Hz
	 *     w2 - Baud rate
	 *     x3 - pointer to empty console_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -----------------------------------------------
	 */
func console_a3700_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_BASE]

	bl	console_a3700_core_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register a3700, putc=1, getc=1, flush=1

register_fail:
	ret	x7
endfunc console_a3700_register

	/* --------------------------------------------------------
	 * int console_a3700_core_putc(int c, unsigned int base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_a3700_core_putc
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
endfunc console_a3700_core_putc

	/* --------------------------------------------------------
	 * int console_a3700_putc(int c, console_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_a3700_putc
	ldr	x1, [x1, #CONSOLE_T_BASE]
	b	console_a3700_core_putc
endfunc console_a3700_putc

	/* ---------------------------------------------
	 * int console_a3700_core_getc(void)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : w0 - console base address
	 * Out : w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_a3700_core_getc
	/* Check if there is a pending character */
	ldr	w1, [x0, #UART_STATUS_REG]
	and	w1, w1, #UARTLSR_RXRDY
	cmp	w1, #UARTLSR_RXRDY
	b.ne	getc_no_char
	ldr	w0, [x0, #UART_RX_REG]
	and	w0, w0, #0xff
	ret
getc_no_char:
	mov	w0, #ERROR_NO_PENDING_CHAR
	ret
endfunc console_a3700_core_getc

	/* ---------------------------------------------
	 * int console_a3700_getc(console_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on if no character is available.
	 * In :  x0 - pointer to console_t structure
	 * Out : w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_a3700_getc
	ldr	x0, [x0, #CONSOLE_T_BASE]
	b	console_a3700_core_getc
endfunc console_a3700_getc

	/* ---------------------------------------------
	 * void console_a3700_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Out : void.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_a3700_core_flush
	/* Wait for the TX (THR and TSR) to be empty */
1:	ldr	w1, [x0, #UART_STATUS_REG]
	and	w1, w1, #UARTLSR_TXEMPTY
	cmp	w1, #UARTLSR_TXEMPTY
	b.ne	1b
	ret
endfunc console_a3700_core_flush

	/* ---------------------------------------------
	 * void console_a3700_flush(console_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - pointer to console_t structure
	 * Out : void.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_a3700_flush
	ldr	x0, [x0, #CONSOLE_T_BASE]
	b	console_a3700_core_flush
endfunc console_a3700_flush

