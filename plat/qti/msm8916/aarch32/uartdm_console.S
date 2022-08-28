/*
 * Copyright (c) 2021-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * Based on aarch32/skeleton_console.S:
 * Copyright (c) 2016-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <console_macros.S>

/* UART DM registers */
#define UART_DM_DMEN		0x03c		/* DMA / data packing */
#define UART_DM_SR		0x0a4		/* status register */
#define UART_DM_CR		0x0a8		/* command register */
#define UART_DM_TF		0x100		/* transmit FIFO */

#define UART_DM_DMEN_TX_SC	BIT_32(4)	/* TX single character mode */

#define UART_DM_SR_TXRDY	BIT_32(2)	/* TX FIFO has space */
#define UART_DM_SR_TXEMT	BIT_32(3)	/* TX FIFO is empty */

#define UART_DM_CR_RESET_RX	(U(0x01) << 4)	/* reset receiver */
#define UART_DM_CR_RESET_TX	(U(0x02) << 4)	/* reset transmitter */
#define UART_DM_CR_TX_ENABLE	BIT_32(2)	/* enable transmitter */

	.globl	console_uartdm_register
	.globl	console_uartdm_core_init
	.globl	console_uartdm_putc
	.globl	console_uartdm_core_putc
	.globl	console_uartdm_flush
	.globl	console_uartdm_core_flush

	/* -----------------------------------------------------------
	 * int console_uartdm_register(console_t *console,
	 * 	uintptr_t base_addr)
	 * Function to initialize and register the console. The caller
	 * needs to pass an empty console_t structure in which *MUST*
	 * be allocated in persistent memory (e.g. a global or static
	 * local variable, *NOT* on the stack).
	 * In : r0 - pointer to empty console_t structure
	 *      r1 - base address
	 * Out: r0 - 1 on success, 0 on error
	 * Clobber list : r0 - r7
	 * -----------------------------------------------------------
	 */
func console_uartdm_register
	str	r1, [r0, #CONSOLE_T_BASE]
	mov	r7, lr
	bl	console_uartdm_core_init
	mov	lr, r7

	/* Register the new console */
	finish_console_register uartdm putc=1, flush=1
endfunc console_uartdm_register

	/* -----------------------------------------------------------
	 * void console_uartdm_core_init(unused, uintptr_t base_addr)
	 * Function to initialize the console.
	 * In : r0 - unused
	 *      r1 - base address
	 * Out: void
	 * Clobber list : r1, r2, r3
	 * -----------------------------------------------------------
	 */
func console_uartdm_core_init
	/*
	 * Try to flush remaining characters from the TX FIFO before resetting
	 * the transmitter. Unfortunately there is no good way to check if
	 * the transmitter is actually enabled (and will finish eventually),
	 * so use a timeout to avoid looping forever.
	 */
	mov	r2, #65536
1:
	ldr	r3, [r1, #UART_DM_SR]
	tst	r3, #UART_DM_SR_TXEMT
	bne	2f
	subs	r2, r2, #1
	bne	1b
	/* Timeout */

2:	/* Reset receiver */
	mov	r3, #UART_DM_CR_RESET_RX
	str	r3, [r1, #UART_DM_CR]

	/* Reset transmitter */
	mov	r3, #UART_DM_CR_RESET_TX
	str	r3, [r1, #UART_DM_CR]

	/*
	 * Disable BAM/DMA modes but enable single-character mode for TX.
	 * The single character mode allows simplifying the putc implementation
	 * since characters can be written directly to the FIFO instead of
	 * having to initiate a new transfer and waiting for its completion.
	 */
	mov	r3, #UART_DM_DMEN_TX_SC
	str	r3, [r1, #UART_DM_DMEN]

	/* Enable transmitter */
	mov	r3, #UART_DM_CR_TX_ENABLE
	str	r3, [r1, #UART_DM_CR]

	bx	lr
endfunc console_uartdm_core_init

	/* -----------------------------------------------------------
	 * int console_uartdm_putc(int c, console_t *console)
	 * Function to output a character over the console.
	 * In : r0 - character to be printed
	 *      r1 - pointer to console_t struct
	 * Out: r0 - printed character on success, < 0 on error.
	 * Clobber list : r0, r1, r2
	 * -----------------------------------------------------------
	 */
func console_uartdm_putc
	ldr	r1, [r1, #CONSOLE_T_BASE]
	b	console_uartdm_core_putc
endfunc console_uartdm_putc

	/* -----------------------------------------------------------
	 * int console_uartdm_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console.
	 * In : r0 - character to be printed
	 *      r1 - base address
	 * Out: r0 - printed character on success, < 0 on error.
	 * Clobber list : r2
	 * -----------------------------------------------------------
	 */
func console_uartdm_core_putc
	cmp	r0, #'\n'
	bne	2f

1:	/* Loop until TX FIFO has space */
	ldr	r2, [r1, #UART_DM_SR]
	tst	r2, #UART_DM_SR_TXRDY
	beq	1b

	/* Prepend '\r' to '\n' */
	mov	r2, #'\r'
	str	r2, [r1, #UART_DM_TF]

2:	/* Loop until TX FIFO has space */
	ldr	r2, [r1, #UART_DM_SR]
	tst	r2, #UART_DM_SR_TXRDY
	beq	2b

	/* Write character to FIFO */
	str	r0, [r1, #UART_DM_TF]
	bx	lr
endfunc console_uartdm_core_putc

	/* -----------------------------------------------------------
	 * void console_uartdm_flush(console_t *console)
	 * Function to force a write of all buffered data
	 * that has not been output.
	 * In : r0 - pointer to console_t struct
	 * Out: void
	 * Clobber list : r0, r1, r2, r3, r4, r5
	 * -----------------------------------------------------------
	 */
func console_uartdm_flush
	ldr	r1, [r0, #CONSOLE_T_BASE]
	b	console_uartdm_core_flush
endfunc console_uartdm_flush

	/* -----------------------------------------------------------
	 * void console_uartdm_core_flush(unused, uintptr_t base_addr)
	 * Function to force a write of all buffered data
	 * that has not been output.
	 * In : r0 - unused
	 *      r1 - base address
	 * Out: void
	 * Clobber list : r2
	 * -----------------------------------------------------------
	 */
func console_uartdm_core_flush
1:	/* Loop until TX FIFO is empty */
	ldr	r2, [r1, #UART_DM_SR]
	tst	r2, #UART_DM_SR_TXEMT
	beq	1b
	bx	lr
endfunc console_uartdm_core_flush
