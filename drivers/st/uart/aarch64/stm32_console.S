/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
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
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x1, x2, x3, x4
	 * -----------------------------------------------
	 */
func console_stm32_core_init
	/* Check the input base address */
	cbz	x0, core_init_fail
#if !defined(IMAGE_BL2)
#if STM32MP_RECONFIGURE_CONSOLE
	/* UART clock rate is set to 0 in BL32, skip init in that case */
	cbz	x1, 1f
#else /* STM32MP_RECONFIGURE_CONSOLE */
	/* Skip UART initialization if it is already enabled */
	ldr	w3, [x0, #USART_CR1]
	tst	w3, #USART_CR1_UE
	b.ne	1f
#endif /* STM32MP_RECONFIGURE_CONSOLE */
#endif /* IMAGE_BL2 */
	/* Check baud rate and uart clock for sanity */
	cbz	w1, core_init_fail
	cbz	w2, core_init_fail
	/* Disable UART */
	ldr	w3, [x0, #USART_CR1]
	mov	w4, #USART_CR1_UE
	bic	w3, w3, w4
	str	w3, [x0, #USART_CR1]
	/* Configure UART */
	mov	w4, #(USART_CR1_TE)
	orr	w4, w4, #(USART_CR1_FIFOEN)
	orr	w3, w3, w4
	str	w3, [x0, #USART_CR1]
	ldr	w3, [x0, #USART_CR2]
	mov	w4, #USART_CR2_STOP
	bic	w3, w3, w4
	str	w3, [x0, #USART_CR2]
	/* Divisor =  (Uart clock + (baudrate / 2)) / baudrate */
	lsr	w3, w2, #1
	add	w3, w1, w3
	udiv	w3, w3, w2
	cmp	w3, #16
	b.hi	2f
	/* Oversampling 8 */
	/* Divisor =  (2 * Uart clock + (baudrate / 2)) / baudrate */
	lsr	w3, w2, #1
	add	w3, w3, w1, lsl #1
	udiv	w3, w3, w2
	and	w1, w3, #USART_BRR_DIV_FRACTION
	lsr	w1, w1, #1
	bic	w3, w3, #USART_BRR_DIV_FRACTION
	orr	w3, w3, w1
	ldr	w1, [x0, #USART_CR1]
	orr	w1, w1, #USART_CR1_OVER8
	str	w1, [x0, #USART_CR1]
2:
	str	w3, [x0, #USART_BRR]
	/* Enable UART */
	ldr	w3, [x0, #USART_CR1]
	mov	w4, #USART_CR1_UE
	orr	w3, w3, w4
	str	w3, [x0, #USART_CR1]
	/* Check TEACK bit */
	mov	w2, #USART_TIMEOUT
teack_loop:
	subs	w2, w2, #1
	beq	core_init_fail
	ldr	w3, [x0, #USART_ISR]
	tst	w3, #USART_ISR_TEACK
	beq	teack_loop
1:
	mov	w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_stm32_core_init

	.globl console_stm32_register

	/* -------------------------------------------------------
	 * int console_stm32_register(uintptr_t baseaddr,
	 *     uint32_t clock, uint32_t baud,
	 *     console_t *console);
	 * Function to initialize and register a new STM32
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: x0 - UART register base address
	 *     w1 - UART clock in Hz
	 *     w2 - Baud rate
	 *     x3 - pointer to empty console_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -------------------------------------------------------
	 */
func console_stm32_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_BASE]

	bl	console_stm32_core_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register stm32 putc=1, getc=0, flush=1

register_fail:
	ret	x7
endfunc console_stm32_register

	/* --------------------------------------------------------
	 * int console_stm32_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_stm32_core_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Check Transmit Data Register Empty */
txe_loop:
	ldr	w2, [x1, #USART_ISR]
	tst	w2, #USART_ISR_TXE
	beq	txe_loop
	str	w0, [x1, #USART_TDR]
	/* Check transmit complete flag */
tc_loop:
	ldr	w2, [x1, #USART_ISR]
	tst	w2, #USART_ISR_TC
	beq	tc_loop
	ret
endfunc console_stm32_core_putc

	/* --------------------------------------------------------
	 * int console_stm32_putc(int c, console_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_stm32_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x1, [x1, #CONSOLE_T_BASE]
	b	console_stm32_core_putc
endfunc console_stm32_putc

	/* ---------------------------------------------
	 * int console_stm32_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : x0 - console base address
	 * Out: w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_stm32_core_getc
	/* Not supported */
	mov	w0, #-1
	ret
endfunc console_stm32_core_getc

	/* ---------------------------------------------
	 * int console_stm32_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_stm32_core_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	/* Check Transmit Data Register Empty */
txe_loop_3:
	ldr	w1, [x0, #USART_ISR]
	tst	w1, #USART_ISR_TXE
	beq	txe_loop_3
	mov	w0, #0
	ret
endfunc console_stm32_core_flush

	/* ---------------------------------------------
	 * int console_stm32_flush(console_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - pointer to console_t structure
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_stm32_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_BASE]
	b	console_stm32_core_flush
endfunc console_stm32_flush
