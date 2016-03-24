/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include "../../../console/aarch32/console.S"

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
	 * In: r0 - console base address
	 *     r1 - Uart clock in Hz
	 *     r2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : r1, r2, r3
	 * -----------------------------------------------
	 */
func console_core_init
	/* Check the input base address */
	cmp	r0, #0
	beq	core_init_fail
#if !PL011_GENERIC_UART
	/* Check baud rate and uart clock for sanity */
	cmp	r1, #0
	beq	core_init_fail
	cmp	r2, #0
	beq	core_init_fail
	/* Disable the UART before initialization */
	ldr	r3, [r0, #UARTCR]
	bic	r3, r3, #PL011_UARTCR_UARTEN
	str	r3, [r0, #UARTCR]
	/* Program the baudrate */
	/* Divisor =  (Uart clock * 4) / baudrate */
	lsl	r1, r1, #2
	udiv	r2, r1, r2
	/* IBRD = Divisor >> 6 */
	lsr	r1, r2, #6
	/* Write the IBRD */
	str	r1, [r0, #UARTIBRD]
	/* FBRD = Divisor & 0x3F */
	and	r1, r2, #0x3f
	/* Write the FBRD */
	str	r1, [r0, #UARTFBRD]
	mov	r1, #PL011_LINE_CONTROL
	str	r1, [r0, #UARTLCR_H]
	/* Clear any pending errors */
	mov	r1, #0
	str	r1, [r0, #UARTECR]
	/* Enable tx, rx, and uart overall */
	ldr	r1, =(PL011_UARTCR_RXE | PL011_UARTCR_TXE | PL011_UARTCR_UARTEN)
	str	r1, [r0, #UARTCR]
#endif
	mov	r0, #1
	bx	lr
core_init_fail:
	mov	r0, #0
	bx	lr
endfunc console_core_init

	/* --------------------------------------------------------
	 * int console_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : r0 - character to be printed
	 *      r1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : r2
	 * --------------------------------------------------------
	 */
func console_core_putc
	/* Check the input parameter */
	cmp	r1, #0
	beq	putc_error
	/* Prepend '\r' to '\n' */
	cmp	r0, #0xA
	bne	2f
1:
	/* Check if the transmit FIFO is full */
	ldr	r2, [r1, #UARTFR]
	tst	r2, #PL011_UARTFR_TXFF_BIT
	beq	1b
	mov	r2, #0xD
	str	r2, [r1, #UARTDR]
2:
	/* Check if the transmit FIFO is full */
	ldr	r2, [r1, #UARTFR]
	tst	r2, #PL011_UARTFR_TXFF_BIT
	beq	2b
	str	r0, [r1, #UARTDR]
	bx	lr
putc_error:
	mov	r0, #-1
	bx	lr
endfunc console_core_putc

	/* ---------------------------------------------
	 * int console_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In : r0 - console base address
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_core_getc
	cmp	r0, #0
	beq	getc_error
1:
	/* Check if the receive FIFO is empty */
	ldr	r1, [r0, #UARTFR]
	tst	r1, #PL011_UARTFR_RXFE_BIT
	beq	1b
	ldr	r1, [r0, #UARTDR]
	mov	r0, r1
	bx	lr
getc_error:
	mov	r0, #-1
	bx	lr
endfunc console_core_getc
