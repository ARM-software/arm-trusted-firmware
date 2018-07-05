/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>

#define USART_TIMEOUT		0x1000

#define USART_CR1		0x00
#define USART_CR1_UE		0x00000001
#define USART_CR1_TE		0x00000008
#define USART_CR1_FIFOEN	0x20000000

#define USART_CR2		0x04
#define USART_CR2_STOP		0x00003000

#define USART_BRR		0x0C

#define USART_ISR		0x1C
#define USART_ISR_TC		0x00000040
#define USART_ISR_TXE		0x00000080
#define USART_ISR_TEACK		0x00200000

#define USART_TDR		0x28

	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc
	.globl	console_core_flush

	/* -----------------------------------------------------------------
	 * int console_core_init(uintptr_t base_addr,
	 *			 unsigned int uart_clk,
	 *			 unsigned int baud_rate)
	 *
	 * Function to initialize the console without a C Runtime to print
	 * debug information. This function will be accessed by console_init
	 * and crash reporting.
	 *
	 * In: r0 - console base address
	 *     r1 - Uart clock in Hz
	 *     r2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : r1, r2, r3
	 * -----------------------------------------------------------------
	 */
func console_core_init
	/* Check the input base address */
	cmp	r0, #0
	beq	core_init_fail
#if defined(IMAGE_BL2)
	/* Check baud rate and uart clock for sanity */
	cmp	r1, #0
	beq	core_init_fail
	cmp	r2, #0
	beq	core_init_fail
	/* Disable UART */
	ldr	r3, [r0, #USART_CR1]
	bic	r3, r3, #USART_CR1_UE
	str	r3, [r0, #USART_CR1]
	/* Configure UART */
	orr	r3, r3, #(USART_CR1_TE | USART_CR1_FIFOEN)
	str	r3, [r0, #USART_CR1]
	ldr	r3, [r0, #USART_CR2]
	bic	r3, r3, #USART_CR2_STOP
	str	r3, [r0, #USART_CR2]
	/* Divisor =  (Uart clock + (baudrate / 2)) / baudrate */
	lsl	r3, r2, #1
	add	r3, r1, r3
	udiv	r3, r3, r2
	str	r3, [r0, #USART_BRR]
	/* Enable UART */
	ldr	r3, [r0, #USART_CR1]
	orr	r3, r3, #USART_CR1_UE
	str	r3, [r0, #USART_CR1]
	/* Check TEACK bit */
	mov	r2, #USART_TIMEOUT
teack_loop:
	subs	r2, r2, #1
	beq	core_init_fail
	ldr	r3, [r0, #USART_ISR]
	tst	r3, #USART_ISR_TEACK
	beq	teack_loop
#endif /* IMAGE_BL2 */
	mov	r0, #1
	bx	lr
core_init_fail:
	mov	r0, #0
	bx	lr
endfunc console_core_init

	/* ---------------------------------------------------------------
	 * int console_core_putc(int c, uintptr_t base_addr)
	 *
	 * Function to output a character over the console. It returns the
	 * character printed on success or -1 on error.
	 *
	 * In : r0 - character to be printed
	 *      r1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : r2
	 * ---------------------------------------------------------------
	 */
func console_core_putc
	/* Check the input parameter */
	cmp	r1, #0
	beq	putc_error
	/* Prepend '\r' to '\n' */
	cmp	r0, #0xA
	bne	2f
1:
	/* Check Transmit Data Register Empty */
txe_loop_1:
	ldr	r2, [r1, #USART_ISR]
	tst	r2, #USART_ISR_TXE
	beq	txe_loop_1
	mov	r2, #0xD
	str	r2, [r1, #USART_TDR]
	/* Check transmit complete flag */
tc_loop_1:
	ldr	r2, [r1, #USART_ISR]
	tst	r2, #USART_ISR_TC
	beq	tc_loop_1
2:
	/* Check Transmit Data Register Empty */
txe_loop_2:
	ldr	r2, [r1, #USART_ISR]
	tst	r2, #USART_ISR_TXE
	beq	txe_loop_2
	str	r0, [r1, #USART_TDR]
	/* Check transmit complete flag */
tc_loop_2:
	ldr	r2, [r1, #USART_ISR]
	tst	r2, #USART_ISR_TC
	beq	tc_loop_2
	bx	lr
putc_error:
	mov	r0, #-1
	bx	lr
endfunc console_core_putc

	/* -----------------------------------------------------------
	 * int console_core_getc(uintptr_t base_addr)
	 *
	 * Function to get a character from the console.
	 * It returns the character grabbed on success or -1 on error.
	 *
	 * In : r0 - console base address
	 * Out : return -1.
	 * Clobber list : r0, r1
	 * -----------------------------------------------------------
	 */
func console_core_getc
	/* Not supported */
	mov	r0, #-1
	bx	lr
endfunc console_core_getc

	/* ---------------------------------------------------------------
	 * int console_core_flush(uintptr_t base_addr)
	 *
	 * Function to force a write of all buffered data that hasn't been
	 * output.
	 *
	 * In : r0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : r0, r1
	 * ---------------------------------------------------------------
	 */
func console_core_flush
	cmp	r0, #0
	beq	flush_error
	/* Check Transmit Data Register Empty */
txe_loop_3:
	ldr	r1, [r0, #USART_ISR]
	tst	r1, #USART_ISR_TXE
	beq	txe_loop_3
	mov	r0, #0
	bx	lr
flush_error:
	mov	r0, #-1
	bx	lr
endfunc console_core_flush
