/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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
#include <pl011.h>

/*
 * Pull in generic functions to provide backwards compatibility for
 * platform makefiles
 */
#include "../../../console/aarch64/console.S"


	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc


	/* -----------------------------------------------
	 * int console_core_init(uintptr_t base_addr,
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
func console_core_init
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
endfunc console_core_init

	/* --------------------------------------------------------
	 * int console_core_putc(int c, uintptr_t base_addr)
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
putc_error:
	mov	w0, #-1
	ret
endfunc console_core_putc

	/* ---------------------------------------------
	 * int console_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In : x0 - console base address
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_core_getc
	cbz	x0, getc_error
1:
	/* Check if the receive FIFO is empty */
	ldr	w1, [x0, #UARTFR]
	tbnz	w1, #PL011_UARTFR_RXFE_BIT, 1b
	ldr	w1, [x0, #UARTDR]
	mov	w0, w1
	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_core_getc
