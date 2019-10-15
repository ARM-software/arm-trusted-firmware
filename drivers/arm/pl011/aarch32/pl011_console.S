/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <console_macros.S>
#include <drivers/arm/pl011.h>

	/*
	 * "core" functions are low-level implementations that don't require
	 * writeable memory and are thus safe to call in BL1 crash context.
	 */
	.globl	console_pl011_core_init
	.globl	console_pl011_core_putc
	.globl	console_pl011_core_getc
	.globl	console_pl011_core_flush

	.globl	console_pl011_putc
	.globl	console_pl011_getc
	.globl	console_pl011_flush


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
func console_pl011_core_init
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
#if (ARM_ARCH_MAJOR == 7) && !defined(ARMV7_SUPPORTS_VIRTUALIZATION)
	push	{r0,r3}
	softudiv	r0,r1,r2,r3
	mov	r2, r0
	pop	{r0,r3}
#else
	udiv	r2, r1, r2
#endif
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
endfunc console_pl011_core_init

	.globl console_pl011_register

	/* -------------------------------------------------------
	 * int console_pl011_register(uintptr_t baseaddr,
	 *     uint32_t clock, uint32_t baud,
	 *     console_pl011_t *console);
	 * Function to initialize and register a new PL011
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: r0 - UART register base address
	 *     r1 - UART clock in Hz
	 *     r2 - Baud rate
	 *     r3 - pointer to empty console_pl011_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : r0, r1, r2
	 * -------------------------------------------------------
	 */
func console_pl011_register
	push	{r4, lr}
	mov	r4, r3
	cmp	r4, #0
	beq	register_fail
	str	r0, [r4, #CONSOLE_T_PL011_BASE]

	bl console_pl011_core_init
	cmp	r0, #0
	beq	register_fail

	mov	r0, r4
	pop	{r4, lr}
	finish_console_register pl011 putc=1, getc=1, flush=1

register_fail:
	pop	{r4, pc}
endfunc console_pl011_register

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
func console_pl011_core_putc
	/* Check the input parameter */
	cmp	r1, #0
	beq	putc_error
	/* Prepend '\r' to '\n' */
	cmp	r0, #0xA
	bne	2f
1:
	/* Check if the transmit FIFO is full */
	ldr	r2, [r1, #UARTFR]
	tst	r2, #PL011_UARTFR_TXFF
	bne	1b
	mov	r2, #0xD
	str	r2, [r1, #UARTDR]
2:
	/* Check if the transmit FIFO is full */
	ldr	r2, [r1, #UARTFR]
	tst	r2, #PL011_UARTFR_TXFF
	bne	2b
	str	r0, [r1, #UARTDR]
	bx	lr
putc_error:
	mov	r0, #-1
	bx	lr
endfunc console_pl011_core_putc

	/* --------------------------------------------------------
	 * int console_pl011_putc(int c, console_pl011_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In: r0 - character to be printed
	 *     r1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list: r2
	 * -------------------------------------------------------
	 */
func console_pl011_putc
#if ENABLE_ASSERTIONS
	cmp	r1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	r1, [r1, #CONSOLE_T_PL011_BASE]
	b	console_pl011_core_putc
endfunc console_pl011_putc

	/* ---------------------------------------------
	 * int console_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In : r0 - console base address
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_pl011_core_getc
	cmp	r0, #0
	beq	getc_error
1:
	/* Check if the receive FIFO is empty */
	ldr	r1, [r0, #UARTFR]
	tst	r1, #PL011_UARTFR_RXFE
	bne	1b
	ldr	r1, [r0, #UARTDR]
	mov	r0, r1
	bx	lr
getc_error:
	mov	r0, #-1
	bx	lr
endfunc console_pl011_core_getc

	/* ------------------------------------------------
	 * int console_pl011_getc(console_pl011_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : r0 - pointer to console_t structure
	 * Out: r0 - character if available, else -1
	 * Clobber list: r0, r1
	 * ------------------------------------------------
	 */
func console_pl011_getc
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	r0, [r0, #CONSOLE_T_PL011_BASE]
	b	console_pl011_core_getc
endfunc console_pl011_getc

	/* ---------------------------------------------
	 * int console_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : r0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : r0, r1
	 * ---------------------------------------------
	 */
func console_pl011_core_flush
	cmp	r0, #0
	beq	flush_error

1:
	/* Loop while the transmit FIFO is busy */
	ldr	r1, [r0, #UARTFR]
	tst	r1, #PL011_UARTFR_BUSY
	bne	1b

	mov	r0, #0
	bx	lr
flush_error:
	mov	r0, #-1
	bx	lr
endfunc console_pl011_core_flush

	/* ---------------------------------------------
	 * int console_pl011_flush(console_pl011_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : r0 - pointer to console_t structure
	 * Out : return -1 on error else return 0.
	 * Clobber list: r0, r1
	 * ---------------------------------------------
	 */
func console_pl011_flush
#if ENABLE_ASSERTIONS
	cmp	r0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	r0, [r0, #CONSOLE_T_PL011_BASE]
	b	console_pl011_core_flush
endfunc console_pl011_flush
