/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <asm_macros.S>
#include <assert_macros.S>
#include <console_macros.S>
#include <drivers/st/stm32_console.h>
#include <drivers/st/stm32_uart_regs.h>

#define USART_TIMEOUT		0x1000

	/*
	 * "core" functions are low-level implementations that don't require
	 * writeable memory and are thus safe to call in BL1 crash context.
	 */
	.globl	console_stm32_core_init
	.globl	console_stm32_core_putc
	.globl	console_stm32_core_getc
	.globl	console_stm32_core_flush

	.globl	console_stm32_putc
	.globl	console_stm32_flush



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
func console_stm32_core_init
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
endfunc console_stm32_core_init

	.globl console_stm32_register

	/* -------------------------------------------------------
	 * int console_stm32_register(uintptr_t baseaddr,
	 *     uint32_t clock, uint32_t baud,
	 *     console_t *console);
	 * Function to initialize and register a new STM32
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: r0 - UART register base address
	 *     r1 - UART clock in Hz
	 *     r2 - Baud rate
	 *     r3 - pointer to empty console_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : r0, r1, r2
	 * -------------------------------------------------------
	 */
func console_stm32_register
	push	{r4, lr}
	mov	r4, r3
	cmp	r4, #0
	beq	register_fail
	str	r0, [r4, #CONSOLE_T_BASE]

	bl console_stm32_core_init
	cmp	r0, #0
	beq	register_fail

	mov	r0, r4
	pop	{r4, lr}
	finish_console_register stm32 putc=1, getc=0, flush=1

register_fail:
	pop	{r4, pc}
endfunc console_stm32_register

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
func console_stm32_core_putc
	/* Check the input parameter */
	cmp	r1, #0
	beq	putc_error

	/* Check Transmit Data Register Empty */
txe_loop:
	ldr	r2, [r1, #USART_ISR]
	tst	r2, #USART_ISR_TXE
	beq	txe_loop
	str	r0, [r1, #USART_TDR]
	/* Check transmit complete flag */
tc_loop:
	ldr	r2, [r1, #USART_ISR]
	tst	r2, #USART_ISR_TC
	beq	tc_loop
	bx	lr
putc_error:
	mov	r0, #-1
	bx	lr
endfunc console_stm32_core_putc

	/* ------------------------------------------------------------
	 * int console_stm32_putc(int c, console_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In: r0 - character to be printed
	 *     r1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list: r2
	 * ------------------------------------------------------------
	 */
func console_stm32_putc
#if ENABLE_ASSERTIONS
	cmp	r1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	r1, [r1, #CONSOLE_T_BASE]
	b	console_stm32_core_putc
endfunc console_stm32_putc

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
func console_stm32_core_getc
	/* Not supported */
	mov	r0, #-1
	bx	lr
endfunc console_stm32_core_getc

	/* ---------------------------------------------------------------
	 * void console_core_flush(uintptr_t base_addr)
	 *
	 * Function to force a write of all buffered data that hasn't been
	 * output.
	 *
	 * In : r0 - console base address
	 * Out : void.
	 * Clobber list : r0, r1
	 * ---------------------------------------------------------------
	 */
func console_stm32_core_flush
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	/* Check Transmit Data Register Empty */
txe_loop_3:
	ldr	r1, [r0, #USART_ISR]
	tst	r1, #USART_ISR_TXE
	beq	txe_loop_3
	bx	lr
endfunc console_stm32_core_flush

	/* ------------------------------------------------------
	 * void console_stm32_flush(console_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : r0 - pointer to console_t structure
	 * Out : void.
	 * Clobber list: r0, r1
	 * ------------------------------------------------------
	 */
func console_stm32_flush
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	r0, [r0, #CONSOLE_T_BASE]
	b	console_stm32_core_flush
endfunc console_stm32_flush
