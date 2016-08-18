/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <asm_macros.S>
#include <uart_16550.h>

	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc

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
init_fail:
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
1:	ldr	w2, [x1, #UARTLSR]
	and	w2, w2, #(UARTLSR_TEMT | UARTLSR_THRE)
	cmp	w2, #(UARTLSR_TEMT | UARTLSR_THRE)
	b.ne	1b
	mov	w2, #0xD		/* '\r' */
	str	w2, [x1, #UARTTX]
	ldr	w2, [x1, #UARTFCR]
	orr	w2, w2, #UARTFCR_TXCLR
	str	w2, [x1, #UARTFCR]

	/* Check if the transmit FIFO is full */
2:	ldr	w2, [x1, #UARTLSR]
	and	w2, w2, #(UARTLSR_TEMT | UARTLSR_THRE)
	cmp	w2, #(UARTLSR_TEMT | UARTLSR_THRE)
	b.ne	2b
	str	w0, [x1, #UARTTX]
	ldr	w2, [x1, #UARTFCR]
	orr	w2, w2, #UARTFCR_TXCLR
	str	w2, [x1, #UARTFCR]
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
1:	ldr	w1, [x0, #UARTLSR]
	tbz	w1, #UARTLSR_RDR, 1b
	ldr	w0, [x0, #UARTRX]
	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_core_getc
