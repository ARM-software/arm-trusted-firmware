/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <console_macros.S>
#include <drivers/arm/pl011.h>

	/*
	 * "core" functions are low-level implementations that don't require
	 * writable memory and are thus safe to call in BL1 crash context.
	 */
	.globl console_pl011_core_init
	.globl console_pl011_core_putc
	.globl console_pl011_core_getc
	.globl console_pl011_core_flush

	.globl	console_pl011_putc
	.globl	console_pl011_getc
	.globl	console_pl011_flush

	/* -----------------------------------------------
	 * int console_pl011_core_init(uintptr_t base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x1, x2, x3, x4
	 * -----------------------------------------------
	 */
func console_pl011_core_init
	/* Check the input base address */
	cbz	x0, core_init_fail
#if !PL011_GENERIC_UART
	/* Check baud rate and uart clock for sanity */
	cbz	w1, core_init_fail
	cbz	w2, core_init_fail
	/* Disable uart before programming */
	ldr	w3, [x0, #UARTCR]
	mov	w4, #PL011_UARTCR_UARTEN
	bic	w3, w3, w4
	str	w3, [x0, #UARTCR]
	/* Program the baudrate */
	/* Divisor =  (Uart clock * 4) / baudrate */
	lsl	w1, w1, #2
	udiv	w2, w1, w2
	/* IBRD = Divisor >> 6 */
	lsr	w1, w2, #6
	/* Write the IBRD */
	str	w1, [x0, #UARTIBRD]
	/* FBRD = Divisor & 0x3F */
	and	w1, w2, #0x3f
	/* Write the FBRD */
	str	w1, [x0, #UARTFBRD]
	mov	w1, #PL011_LINE_CONTROL
	str	w1, [x0, #UARTLCR_H]
	/* Clear any pending errors */
	str	wzr, [x0, #UARTECR]
	/* Enable tx, rx, and uart overall */
	mov	w1, #(PL011_UARTCR_RXE | PL011_UARTCR_TXE | PL011_UARTCR_UARTEN)
	str	w1, [x0, #UARTCR]
#endif
	mov	w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_pl011_core_init

	.globl console_pl011_register

	/* -----------------------------------------------
	 * int console_pl011_register(uintptr_t baseaddr,
	 *     uint32_t clock, uint32_t baud,
	 *     console_pl011_t *console);
	 * Function to initialize and register a new PL011
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: x0 - UART register base address
	 *     w1 - UART clock in Hz
	 *     w2 - Baud rate
	 *     x3 - pointer to empty console_pl011_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -----------------------------------------------
	 */
func console_pl011_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_PL011_BASE]

	bl	console_pl011_core_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register pl011 putc=1, getc=1, flush=1

register_fail:
	ret	x7
endfunc console_pl011_register

	/* --------------------------------------------------------
	 * int console_pl011_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_pl011_core_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Prepend '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
1:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #UARTFR]
	tbnz	w2, #PL011_UARTFR_TXFF_BIT, 1b
	mov	w2, #0xD
	str	w2, [x1, #UARTDR]
2:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #UARTFR]
	tbnz	w2, #PL011_UARTFR_TXFF_BIT, 2b
	str	w0, [x1, #UARTDR]
	ret
endfunc console_pl011_core_putc

	/* --------------------------------------------------------
	 * int console_pl011_putc(int c, console_pl011_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_pl011_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x1, [x1, #CONSOLE_T_PL011_BASE]
	b	console_pl011_core_putc
endfunc console_pl011_putc

	/* ---------------------------------------------
	 * int console_pl011_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : x0 - console base address
	 * Out: w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_pl011_core_getc
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Check if the receive FIFO is empty */
	ldr	w1, [x0, #UARTFR]
	tbnz	w1, #PL011_UARTFR_RXFE_BIT, no_char
	ldr	w1, [x0, #UARTDR]
	mov	w0, w1
	ret
no_char:
	mov	w0, #ERROR_NO_PENDING_CHAR
	ret
endfunc console_pl011_core_getc

	/* ---------------------------------------------
	 * int console_pl011_getc(console_pl011_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : x0 - pointer to console_t structure
	 * Out: w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_pl011_getc
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_PL011_BASE]
	b	console_pl011_core_getc
endfunc console_pl011_getc

	/* ---------------------------------------------
	 * int console_pl011_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_pl011_core_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
1:
	/* Loop until the transmit FIFO is empty */
	ldr	w1, [x0, #UARTFR]
	tbnz	w1, #PL011_UARTFR_BUSY_BIT, 1b

	mov	w0, #0
	ret
endfunc console_pl011_core_flush

	/* ---------------------------------------------
	 * int console_pl011_flush(console_pl011_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - pointer to console_t structure
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_pl011_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_PL011_BASE]
	b	console_pl011_core_flush
endfunc console_pl011_flush
