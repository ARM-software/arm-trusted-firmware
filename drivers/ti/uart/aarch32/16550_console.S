/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <console_macros.S>
#include <drivers/ti/uart/uart_16550.h>

	/*
	 * "core" functions are low-level implementations that don't require
	 * writable memory and are thus safe to call in BL1 crash context.
	 */
	.globl console_16550_core_init
	.globl console_16550_core_putc
	.globl console_16550_core_getc
	.globl console_16550_core_flush

	.globl console_16550_putc
	.globl console_16550_getc
	.globl console_16550_flush

	/* -----------------------------------------------
	 * int console_16550_core_init(uintptr_t base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: r0 - console base address
	 *     r1 - Uart clock in Hz
	 *     r2 - Baud rate
	 * Out: return 1 on success, 0 on error
	 * Clobber list : r1, r2, r3
	 * -----------------------------------------------
	 */
func console_16550_core_init
	/* Check the input base address */
	cmp	r0, #0
	beq	init_fail
	/* Check baud rate and uart clock for sanity */
	cmp	r1, #0
	beq	init_fail
	cmp	r2, #0
	beq	init_fail

	/* Program the baudrate */
	/* Divisor =  Uart clock / (16 * baudrate) */
	lsl	r2, r2, #4
	udiv	r2, r1, r2
	and	r1, r2, #0xff		/* w1 = DLL */
	lsr	r2, r2, #8
	and	r2, r2, #0xff		/* w2 = DLLM */
	ldr	r3, [r0, #UARTLCR]
	orr	r3, r3, #UARTLCR_DLAB
	str	r3, [r0, #UARTLCR]	/* enable DLL, DLLM programming */
	str	r1, [r0, #UARTDLL]	/* program DLL */
	str	r2, [r0, #UARTDLLM]	/* program DLLM */
	mov	r2, #~UARTLCR_DLAB
	and	r3, r3, r2
	str	r3, [r0, #UARTLCR]	/* disable DLL, DLLM programming */

	/* 8n1 */
	mov	r3, #3
	str	r3, [r0, #UARTLCR]
	/* no interrupt */
	mov	r3, #0
	str	r3, [r0, #UARTIER]
#ifdef TI_16550_MDR_QUIRK
	/* UART must be enabled on some platforms via the MDR register */
	str	r3, [r0, #UARTMDR1]
#endif /* TI_16550_MDR_QUIRK */
	/* enable fifo, DMA */
	mov	r3, #(UARTFCR_FIFOEN | UARTFCR_DMAEN)
	str	r3, [r0, #UARTFCR]
	/* DTR + RTS */
	mov	r3, #3
	str	r3, [r0, #UARTMCR]
	mov	r0, #1
	bx	lr
init_fail:
	mov	r0, #0
	bx	lr
endfunc console_16550_core_init

	.globl console_16550_register

	/* -------------------------------------------------------
	 * int console_stm32_register(uintptr_t baseaddr,
	 *     uint32_t clock, uint32_t baud,
	 *     struct console_stm32 *console);
	 * Function to initialize and register a new STM32
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: r0 - UART register base address
	 *     r1 - UART clock in Hz
	 *     r2 - Baud rate
	 *     r3 - pointer to empty console_stm32 struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : r0, r1, r2
	 * -------------------------------------------------------
	 */
func console_16550_register
	push	{r4, lr}
	mov	r4, r3
	cmp	r4, #0
	beq	register_fail
	str	r0, [r4, #CONSOLE_T_16550_BASE]

	bl	console_16550_core_init
	cmp	r0, #0
	beq	register_fail

	mov	r0, r4
	pop	{r4, lr}
	finish_console_register 16550 putc=1, getc=1, flush=1

register_fail:
	pop	{r4, pc}
endfunc console_16550_register

	/* --------------------------------------------------------
	 * int console_16550_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : r0 - character to be printed
	 *      r1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : r2
	 * --------------------------------------------------------
	 */
func console_16550_core_putc
#if ENABLE_ASSERTIONS
	cmp	r1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Prepend '\r' to '\n' */
	cmp	r0, #0xA
	bne	2f
	/* Check if the transmit FIFO is full */
1:	ldr	r2, [r1, #UARTLSR]
	and	r2, r2, #(UARTLSR_TEMT | UARTLSR_THRE)
	cmp	r2, #(UARTLSR_TEMT | UARTLSR_THRE)
	bne	1b
	mov	r2, #0xD		/* '\r' */
	str	r2, [r1, #UARTTX]

	/* Check if the transmit FIFO is full */
2:	ldr	r2, [r1, #UARTLSR]
	and	r2, r2, #(UARTLSR_TEMT | UARTLSR_THRE)
	cmp	r2, #(UARTLSR_TEMT | UARTLSR_THRE)
	bne	2b
	str	r0, [r1, #UARTTX]
	bx	lr
endfunc console_16550_core_putc

	/* --------------------------------------------------------
	 * int console_16550_putc(int c, console_16550_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : r0 - character to be printed
	 *      r1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list : r2
	 * --------------------------------------------------------
	 */
func console_16550_putc
#if ENABLE_ASSERTIONS
	cmp	r1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	r1, [r1, #CONSOLE_T_16550_BASE]
	b	console_16550_core_putc
endfunc console_16550_putc

	/* ---------------------------------------------
	 * int console_16550_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on if no character is available.
	 * In :  r0 - console base address
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_16550_core_getc
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Check if the receive FIFO is empty */
1:	ldr	r1, [r0, #UARTLSR]
	tst	r1, #UARTLSR_RDR_BIT
	beq	no_char
	ldr	r1, [r0, #UARTRX]
	mov	r0, r1
	bx	lr
no_char:
	mov	r0, #ERROR_NO_PENDING_CHAR
	bx	lr
endfunc console_16550_core_getc

	/* ---------------------------------------------
	 * int console_16550_getc(console_16550_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on if no character is available.
	 * In :  r0 - pointer to console_t stucture
	 * Out : r0 - character if available, else -1
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_16550_getc
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	r0, [r0, #CONSOLE_T_16550_BASE]
	b	console_16550_core_getc
endfunc console_16550_getc

	/* ---------------------------------------------
	 * int console_16550_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : r0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_16550_core_flush
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* Loop until the transmit FIFO is empty */
1:	ldr	r1, [r0, #UARTLSR]
	and	r1, r1, #(UARTLSR_TEMT | UARTLSR_THRE)
	cmp	r1, #(UARTLSR_TEMT | UARTLSR_THRE)
	bne	1b

	mov	r0, #0
	bx	lr
endfunc console_16550_core_flush

	/* ---------------------------------------------
	 * int console_16550_flush(console_pl011_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : r0 - pointer to console_t structure
	 * Out : return -1 on error else return 0.
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_16550_flush
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	r0, [r0, #CONSOLE_T_16550_BASE]
	b	console_16550_core_flush
endfunc console_16550_flush
