/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <console_macros.S>
#include <assert_macros.S>
#include "ls_16550.h"

	/*
	 * "core" functions are low-level implementations that don't require
	 * writable memory and are thus safe to call in BL1 crash context.
	 */
	.globl console_ls_16550_core_init
	.globl console_ls_16550_core_putc
	.globl console_ls_16550_core_getc

	.globl console_ls_16550_putc
	.globl console_ls_16550_getc
	.globl console_ls_16550_flush

	/* -----------------------------------------------
	 * int console_ls_16550_core_init(uintptr_t base_addr,
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
func console_ls_16550_core_init
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
	ldrb	w3, [x0, #UARTLCR]
	orr	w3, w3, #UARTLCR_DLAB
	strb	w3, [x0, #UARTLCR]	/* enable DLL, DLLM programming */
	strb	w1, [x0, #UARTDLL]	/* program DLL */
	strb	w2, [x0, #UARTDLLM]	/* program DLLM */
	mov	w2, #~UARTLCR_DLAB
	and	w3, w3, w2
	strb	w3, [x0, #UARTLCR]	/* disable DLL, DLLM programming */

	/* 8n1 */
	mov	w3, #3
	strb	w3, [x0, #UARTLCR]
	/* no interrupt */
	mov	w3, #0
	strb	w3, [x0, #UARTIER]
	/* enable fifo, DMA */
	mov	w3, #(UARTFCR_FIFOEN |UARTFCR_TXCLR | UARTFCR_RXCLR)
	strb	w3, [x0, #UARTFCR]
	/* DTR + RTS */
	mov	w3, #3
	str	w3, [x0, #UARTMCR]
	mov	w0, #1
	ret
init_fail:
	mov	w0, #0
	ret
endfunc console_ls_16550_core_init

	.globl console_ls_16550_register

	/* -----------------------------------------------
	 * int console_ls_16550_register(console_ls_16550_t *console,
	 *	uintptr_t base, uint32_t clk, uint32_t baud)
	 * Function to initialize and register a new 16550
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: x0 - UART register base address
	 *     w1 - UART clock in Hz
	 *     w2 - Baud rate
	 *     x3 - pointer to empty console_ls_16550_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -----------------------------------------------
	 */
func console_ls_16550_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_16550_BASE]

	bl	console_ls_16550_core_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register ls_16550 putc=1, getc=1, flush=1

register_fail:
	ret	x7
endfunc console_ls_16550_register

	/* --------------------------------------------------------
	 * int console_ls_16550_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_ls_16550_core_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Prepend '\r' to '\n' */
	cmp	w0, #0xA //'\n'
	b.ne	2f
	/* Check if the transmit FIFO is full */
1:	ldrb	w2, [x1, #UARTLSR]
	and	w2, w2, #UARTLSR_THRE        /* #(UARTLSR_TEMT | UARTLSR_THRE)*/
	cmp	w2, #(UARTLSR_THRE)
	b.ne	1b
	mov	w2, #0xD		/* '\r' */
	strb	w2, [x1, #UARTTX]
	ldrb	w2, [x1, #UARTFCR]
	orr	w2, w2, #UARTFCR_TXCLR

	/* Check if the transmit FIFO is full */
2:	ldrb	w2, [x1, #UARTLSR]
	and	w2, w2, #(UARTLSR_THRE)
	cmp	w2, #(UARTLSR_THRE)
	b.ne	2b
	strb	w0, [x1, #UARTTX]
	ret
endfunc console_ls_16550_core_putc

	/* --------------------------------------------------------
	 * int console_16550_putc(int c, console_ls_16550_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_ls_16550_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x1, [x1, #CONSOLE_T_16550_BASE]
	b	console_ls_16550_core_putc
endfunc console_ls_16550_putc

	/* ---------------------------------------------
	 * int console_ls_16550_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on if no character is available.
	 * In :  x0 - console base address
	 * Out : w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_ls_16550_core_getc
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Check if the receive FIFO is empty */
1:	ldrb	w1, [x0, #UARTLSR]
	tbz	w1, #UARTLSR_RDR, 1b
	ldrb	w0, [x0, #UARTRX]
	ret
no_char:
	mov	w0, #ERROR_NO_PENDING_CHAR
	ret
endfunc console_ls_16550_core_getc

	/* ---------------------------------------------
	 * int console_ls_16550_getc(console_ls_16550_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on if no character is available.
	 * In :  x0 - pointer to console_t structure
	 * Out : w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_ls_16550_getc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_16550_BASE]
	b	console_ls_16550_core_getc
endfunc console_ls_16550_getc

	/* ---------------------------------------------
	 * int console_ls_16550_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_ls_16550_core_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Loop until the transmit FIFO is empty */
1:	ldrb	w1, [x0, #UARTLSR]
	and	w1, w1, #(UARTLSR_TEMT | UARTLSR_THRE)
	cmp	w1, #(UARTLSR_TEMT | UARTLSR_THRE)
	b.ne	1b

	mov	w0, #0
	ret
endfunc console_ls_16550_core_flush

	/* ---------------------------------------------
	 * int console_ls_16550_flush(console_ls_16550_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - pointer to console_t structure
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_ls_16550_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_16550_BASE]
	b	console_ls_16550_core_flush
endfunc console_ls_16550_flush
