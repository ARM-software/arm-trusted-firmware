/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * Based on aarch64/skeleton_console.S:
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
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

#define UART_DM_SR_TXRDY_BIT	2		/* TX FIFO has space */
#define UART_DM_SR_TXEMT_BIT	3		/* TX FIFO is empty */

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
	 * In : x0 - pointer to empty console_t structure
	 *      x1 - base address
	 * Out: x0 - 1 on success, 0 on error
	 * Clobber list : x0 - x7
	 * -----------------------------------------------------------
	 */
func console_uartdm_register
	str	x1, [x0, #CONSOLE_T_BASE]
	mov	x7, lr
	bl	console_uartdm_core_init
	mov	lr, x7

	/* Register the new console */
	finish_console_register uartdm putc=1, flush=1
endfunc console_uartdm_register

	/* -----------------------------------------------------------
	 * void console_uartdm_core_init(unused, uintptr_t base_addr)
	 * Function to initialize the console.
	 * In : x0 - unused
	 *      x1 - base address
	 * Out: void
	 * Clobber list : x1, x2, x3
	 * -----------------------------------------------------------
	 */
func console_uartdm_core_init
	/* Reset receiver */
	mov	w3, #UART_DM_CR_RESET_RX
	str	w3, [x1, #UART_DM_CR]

	/* Reset transmitter */
	mov	w3, #UART_DM_CR_RESET_TX
	str	w3, [x1, #UART_DM_CR]

	/*
	 * Disable BAM/DMA modes but enable single-character mode for TX.
	 * The single character mode allows simplifying the putc implementation
	 * since characters can be written directly to the FIFO instead of
	 * having to initiate a new transfer and waiting for its completion.
	 */
	mov	w3, #UART_DM_DMEN_TX_SC
	str	w3, [x1, #UART_DM_DMEN]

	/* Enable transmitter */
	mov	w3, #UART_DM_CR_TX_ENABLE
	str	w3, [x1, #UART_DM_CR]

	ret
endfunc console_uartdm_core_init

	/* -----------------------------------------------------------
	 * int console_uartdm_putc(int c, console_t *console)
	 * Function to output a character over the console.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t struct
	 * Out: w0 - printed character on success, < 0 on error.
	 * Clobber list : x0, x1, x2
	 * -----------------------------------------------------------
	 */
func console_uartdm_putc
	ldr	x1, [x1, #CONSOLE_T_BASE]
	b	console_uartdm_core_putc
endfunc console_uartdm_putc

	/* -----------------------------------------------------------
	 * int console_uartdm_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console.
	 * In : w0 - character to be printed
	 *      x1 - base address
	 * Out: w0 - printed character on success, < 0 on error.
	 * Clobber list : x2
	 * -----------------------------------------------------------
	 */
func console_uartdm_core_putc
1:	/* Loop until TX FIFO has space */
	ldr	w2, [x1, #UART_DM_SR]
	tbz	w2, #UART_DM_SR_TXRDY_BIT, 1b

	/* Write character to FIFO */
	str	w0, [x1, #UART_DM_TF]
	ret
endfunc console_uartdm_core_putc

	/* -----------------------------------------------------------
	 * void console_uartdm_flush(console_t *console)
	 * Function to force a write of all buffered data
	 * that has not been output.
	 * In : x0 - pointer to console_t struct
	 * Out: void
	 * Clobber list : x0, x1, x2, x3, x4, x5
	 * -----------------------------------------------------------
	 */
func console_uartdm_flush
	ldr	x1, [x0, #CONSOLE_T_BASE]
	b	console_uartdm_core_flush
endfunc console_uartdm_flush

	/* -----------------------------------------------------------
	 * void console_uartdm_core_flush(unused, uintptr_t base_addr)
	 * Function to force a write of all buffered data
	 * that has not been output.
	 * In : x0 - unused
	 *      x1 - base address
	 * Out: void
	 * Clobber list : x2
	 * -----------------------------------------------------------
	 */
func console_uartdm_core_flush
1:	/* Loop until TX FIFO is empty */
	ldr	w2, [x1, #UART_DM_SR]
	tbz	w2, #UART_DM_SR_TXEMT_BIT, 1b
	ret
endfunc console_uartdm_core_flush
