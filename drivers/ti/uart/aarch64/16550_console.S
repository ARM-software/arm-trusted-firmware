/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <uart_16550.h>

	.globl console_16550_init
	.globl console_16550_putc
	.globl console_16550_getc
	.globl console_16550_flush

#if MULTI_CONSOLE_API
	.globl console_16550_register

	/* -----------------------------------------------
	 * int console_16550_register(console_16550_t *console,
		uintptr_t base, uint32_t clk, uint32_t baud)
	 * Function to initialize and register a new 16550
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: x0 - pointer to empty console_16550_t struct
	 *     x1 - UART register base address
	 *     w2 - UART clock in Hz
	 *     w3 - Baud rate
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -----------------------------------------------
	 */
func console_16550_register
	mov	x7, x30
	cbz	x0, register_fail
	str	x1, [x0, #CONSOLE_T_16550_BASE]

	/* shift registers around to match ABI of legacy init function */
	mov	x6, x0
	mov	x0, x1
	mov	w1, w2
	mov	w2, w3

	bl	console_16550_init
	cbz	x0, register_fail

	mov	x0, x6
	adrp	x1, console_16550_putc
	add	x1, x1, :lo12:console_16550_putc
	str	x1, [x0, #CONSOLE_T_PUTC]
	adrp	x1, console_16550_getc
	add	x1, x1, :lo12:console_16550_getc
	str	x1, [x0, #CONSOLE_T_GETC]
	mov	x1, #0
	str	x1, [x0, #CONSOLE_T_FLUSH]
	mov	x1, #(1 << CONSOLE_FLAG_BOOT_SHIFT | \
		      1 << CONSOLE_FLAG_CRASH_SHIFT)
	str	x1, [x0, #CONSOLE_T_FLAGS]
	bl	console_register
register_fail:
	ret	x7
endfunc console_16550_register
#else
	.globl console_core_init
	.globl console_core_putc
	.globl console_core_getc
	.globl console_core_flush
	.equ console_core_init,console_16550_init
	.equ console_core_putc,console_16550_putc
	.equ console_core_getc,console_16550_getc
#endif

	/* -----------------------------------------------
	 * int console_16550_init(unsigned long base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x1, x2, x3
	 * -----------------------------------------------
	 */
func console_16550_init
	/* Check the input base address */
	cbz	x0, init_fail
	/* Check baud rate and uart clock for sanity */
	cbz	w1, init_fail
	cbz	w2, init_fail

	/* Program the baudrate */
	/* Divisor =  Uart clock / (16 * baudrate) */
	lsl	w2, w2, #4
	udiv	w2, w1, w2
	and	w1, w2, #0xff		/* w1 = DLL */
	lsr	w2, w2, #8
	and	w2, w2, #0xff		/* w2 = DLLM */
	ldr	w3, [x0, #UARTLCR]
	orr	w3, w3, #UARTLCR_DLAB
	str	w3, [x0, #UARTLCR]	/* enable DLL, DLLM programming */
	str	w1, [x0, #UARTDLL]	/* program DLL */
	str	w2, [x0, #UARTDLLM]	/* program DLLM */
	mov	w2, #~UARTLCR_DLAB
	and	w3, w3, w2
	str	w3, [x0, #UARTLCR]	/* disable DLL, DLLM programming */

	/* 8n1 */
	mov	w3, #3
	str	w3, [x0, #UARTLCR]
	/* no interrupt */
	mov	w3, #0
	str	w3, [x0, #UARTIER]
	/* enable fifo, DMA */
	mov	w3, #(UARTFCR_FIFOEN | UARTFCR_DMAEN)
	str	w3, [x0, #UARTFCR]
	/* DTR + RTS */
	mov	w3, #3
	str	w3, [x0, #UARTMCR]
	mov	w0, #1
	ret
init_fail:
	mov	w0, #0
	ret
endfunc console_16550_init

	/* --------------------------------------------------------
	 * int console_16550_putc(int c, console_16550_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_16550_putc
#if MULTI_CONSOLE_API
	ldr	x1, [x1, #CONSOLE_T_16550_BASE]
#endif
	/* Check the input parameter */
	cbz	x1, putc_error
	/* Prepend '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
	/* Check if the transmit FIFO is full */
1:	ldr	w2, [x1, #UARTLSR]
	and	w2, w2, #(UARTLSR_TEMT | UARTLSR_THRE)
	cmp	w2, #(UARTLSR_TEMT | UARTLSR_THRE)
	b.ne	1b
	mov	w2, #0xD		/* '\r' */
	str	w2, [x1, #UARTTX]

	/* Check if the transmit FIFO is full */
2:	ldr	w2, [x1, #UARTLSR]
	and	w2, w2, #(UARTLSR_TEMT | UARTLSR_THRE)
	cmp	w2, #(UARTLSR_TEMT | UARTLSR_THRE)
	b.ne	2b
	str	w0, [x1, #UARTTX]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_16550_putc

	/* ---------------------------------------------
	 * int console_16550_getc(console_16550_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on if no character is available.
	 * In :  x0 - pointer to console_t stucture
	 * Out : w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_16550_getc
#if MULTI_CONSOLE_API
	ldr	x0, [x0, #CONSOLE_T_16550_BASE]
#endif
	/* Check if the receive FIFO is empty */
1:	ldr	w1, [x0, #UARTLSR]
	tbz	w1, #UARTLSR_RDR_BIT, no_char
	ldr	w0, [x0, #UARTRX]
	ret
no_char:
	mov	w0, #-1
	ret
endfunc console_16550_getc

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
