/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <asm_macros.S>
#include <uart8250.h>

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
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x1, x2, x3
	 * -----------------------------------------------
	 */
func console_core_init
	/* Check the input base address */
	cbz	x0, core_init_fail
	/* Check baud rate and uart clock for sanity */
	cbz	w1, core_init_fail
	cbz	w2, core_init_fail

	/* Disable interrupt */
	str	wzr, [x0, #UART_IER]

	/* Force DTR and RTS to high */
	mov	w3, #(UART_MCR_DTR | UART_MCR_RTS)
	str	w3, [x0, #UART_MCR]

	/* Check high speed */
	movz	w3, #:abs_g1:115200
	movk	w3, #:abs_g0_nc:115200
	cmp	w2, w3
	b.hi	1f

	/* Non high speed */
	lsl	w2, w2, #4
	mov	w3, wzr
	b	2f

	/* High speed */
1:	lsl	w2, w2, #2
	mov	w3, #2

	/* Set high speed UART register */
2:	str	w3, [x0, #UART_HIGHSPEED]

	/* Calculate divisor */
	udiv	w3, w1, w2	/* divisor = uartclk / (quot * baudrate) */
	msub	w1, w3, w2, w1	/* remainder = uartclk % (quot * baudrate) */
	lsr	w2, w2, #1
	cmp	w1, w2
	cinc	w3, w3, hs

	/* Set line configuration, access divisor latches */
	mov	w1, #(UART_LCR_DLAB | UART_LCR_WLS_8)
	str	w1, [x0, #UART_LCR]

	/* Set the divisor */
	and	w1, w3, #0xff
	str	w1, [x0, #UART_DLL]
	lsr	w1, w3, #8
	and	w1, w1, #0xff
	str	w1, [x0, #UART_DLH]

	/* Hide the divisor latches */
	mov	w1, #UART_LCR_WLS_8
	str	w1, [x0, #UART_LCR]

	/* Enable FIFOs, and clear receive and transmit */
	mov	w1, #(UART_FCR_FIFO_EN | UART_FCR_CLEAR_RCVR |	\
			UART_FCR_CLEAR_XMIT)
	str	w1, [x0, #UART_FCR]

	mov	w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_core_init

	/* --------------------------------------------------------
	 * int console_core_putc(int c, unsigned long base_addr)
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
1:	ldr	w2, [x1, #UART_LSR]
	and	w2, w2, #UART_LSR_THRE
	cbz	w2, 1b
	mov	w2, #0xD
	str	w2, [x1, #UART_THR]

	/* Check if the transmit FIFO is full */
2:	ldr	w2, [x1, #UART_LSR]
	and	w2, w2, #UART_LSR_THRE
	cbz	w2, 2b
	str	w0, [x1, #UART_THR]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_core_putc

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
	cbz	x0, getc_error

	/* Check if the receive FIFO is empty */
1:	ldr	w1, [x0, #UART_LSR]
	tbz	w1, #UART_LSR_DR, 1b
	ldr	w0, [x0, #UART_RBR]
	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_core_getc
