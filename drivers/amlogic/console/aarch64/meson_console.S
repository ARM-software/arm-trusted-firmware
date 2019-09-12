/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <console_macros.S>
#include <drivers/amlogic/meson_console.h>

	.globl console_meson_register
	.globl console_meson_init
	.globl console_meson_putc
	.globl console_meson_getc
	.globl console_meson_flush
	.globl console_meson_core_putc
	.globl console_meson_core_getc
	.globl console_meson_core_flush

	/* -----------------------------------------------
	 * Hardware definitions
	 * -----------------------------------------------
	 */
#define MESON_WFIFO_OFFSET			0x0
#define MESON_RFIFO_OFFSET			0x4
#define MESON_CONTROL_OFFSET			0x8
#define MESON_STATUS_OFFSET			0xC
#define MESON_MISC_OFFSET			0x10
#define MESON_REG5_OFFSET			0x14

#define MESON_CONTROL_CLR_ERROR_BIT		24
#define MESON_CONTROL_RX_RESET_BIT		23
#define MESON_CONTROL_TX_RESET_BIT		22
#define MESON_CONTROL_RX_ENABLE_BIT		13
#define MESON_CONTROL_TX_ENABLE_BIT		12

#define MESON_STATUS_RX_EMPTY_BIT		20
#define MESON_STATUS_TX_FULL_BIT		21
#define MESON_STATUS_TX_EMPTY_BIT		22

#define MESON_REG5_USE_XTAL_CLK_BIT		24
#define MESON_REG5_USE_NEW_RATE_BIT		23
#define MESON_REG5_NEW_BAUD_RATE_MASK		0x7FFFFF

	/* -----------------------------------------------
	 * int console_meson_register(uintptr_t base,
	 *     uint32_t clk, uint32_t baud,
	 *     console_meson_t *console);
	 * Function to initialize and register a new MESON
	 * console. Storage passed in for the console struct
	 * *must* be persistent (i.e. not from the stack).
	 * In: x0 - UART register base address
	 *     w1 - UART clock in Hz
	 *     w2 - Baud rate
	 *     x3 - pointer to empty console_meson_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7, x14
	 * -----------------------------------------------
	 */
func console_meson_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_MESON_BASE]

	bl	console_meson_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register meson putc=1, getc=1, flush=1

register_fail:
	ret	x7
endfunc console_meson_register

	/* -----------------------------------------------
	 * int console_meson_init(uintptr_t base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x0-x3
	 * -----------------------------------------------
	 */
func console_meson_init
	cmp	w0, #0
	beq	init_fail
	mov_imm	w3, 24000000 /* TODO: This only works with a 24 MHz clock. */
	cmp	w1, w3
	bne	init_fail
	cmp	w2, #0
	beq	init_fail
	/* Set baud rate: value = ((clock / 3) / baudrate) - 1 */
	mov	w3, #3
	udiv	w3, w1, w3
	udiv	w3, w3, w2
	sub	w3, w3, #1
	orr	w3, w3, #((1 << MESON_REG5_USE_XTAL_CLK_BIT) | \
			  (1 << MESON_REG5_USE_NEW_RATE_BIT))
	str	w3, [x0, #MESON_REG5_OFFSET]
	/* Reset UART and clear error flag */
	ldr	w3, [x0, #MESON_CONTROL_OFFSET]
	orr	w3, w3, #((1 << MESON_CONTROL_CLR_ERROR_BIT) | \
			  (1 << MESON_CONTROL_RX_RESET_BIT) | \
			  (1 << MESON_CONTROL_TX_RESET_BIT))
	str	w3, [x0, #MESON_CONTROL_OFFSET]
	bic	w3, w3, #((1 << MESON_CONTROL_CLR_ERROR_BIT) | \
			  (1 << MESON_CONTROL_RX_RESET_BIT) | \
			  (1 << MESON_CONTROL_TX_RESET_BIT))
	str	w3, [x0, #MESON_CONTROL_OFFSET]
	/* Enable transfer and receive FIFO */
	orr	w3, w3, #((1 << MESON_CONTROL_RX_ENABLE_BIT) | \
			  (1 << MESON_CONTROL_TX_ENABLE_BIT))
	str	w3, [x0, #MESON_CONTROL_OFFSET]
	/* Success */
	mov	w0, #1
	ret
init_fail:
	mov	w0, wzr
	ret
endfunc console_meson_init

	/* --------------------------------------------------------
	 * int console_meson_putc(int c, console_meson_t *console)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - pointer to console_t structure
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_meson_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x1, [x1, #CONSOLE_T_MESON_BASE]
	b	console_meson_core_putc
endfunc console_meson_putc

	/* --------------------------------------------------------
	 * int console_meson_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_meson_core_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif
	/* Prepend '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
	/* Wait until the transmit FIFO isn't full */
1:	ldr	w2, [x1, #MESON_STATUS_OFFSET]
	tbnz	w2, #MESON_STATUS_TX_FULL_BIT, 1b
	/* Write '\r' if needed */
	mov	w2, #0xD
	str	w2, [x1, #MESON_WFIFO_OFFSET]
	/* Wait until the transmit FIFO isn't full */
2:	ldr	w2, [x1, #MESON_STATUS_OFFSET]
	tbnz	w2, #MESON_STATUS_TX_FULL_BIT, 2b
	/* Write input character */
	str	w0, [x1, #MESON_WFIFO_OFFSET]
	ret
endfunc console_meson_core_putc

	/* ---------------------------------------------
	 * int console_meson_getc(console_meson_t *console)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : x0 - pointer to console_t structure
	 * Out: w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_meson_getc
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_MESON_BASE]
	b	console_meson_core_getc
endfunc console_meson_getc

	/* ---------------------------------------------
	 * int console_meson_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 if no character is available.
	 * In : x0 - console base address
	 * Out: w0 - character if available, else -1
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_meson_core_getc
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif
	/* Is the receive FIFO empty? */
	ldr	w1, [x0, #MESON_STATUS_OFFSET]
	tbnz	w1, #MESON_STATUS_RX_EMPTY_BIT, 1f
	/* Read one character from the RX FIFO */
	ldr	w0, [x0, #MESON_RFIFO_OFFSET]
	ret
1:
	mov	w0, #ERROR_NO_PENDING_CHAR
	ret
endfunc console_meson_core_getc

	/* ---------------------------------------------
	 * int console_meson_flush(console_meson_t *console)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - pointer to console_t structure
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_meson_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	ldr	x0, [x0, #CONSOLE_T_MESON_BASE]
	b	console_meson_core_flush
endfunc console_meson_flush

	/* ---------------------------------------------
	 * int console_meson_core_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In : x0 - console base address
	 * Out : return -1 on error else return 0.
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_meson_core_flush
#if ENABLE_ASSERTIONS
	cmp	x0, #0
	ASM_ASSERT(ne)
#endif
	/* Wait until the transmit FIFO is empty */
1:	ldr	w1, [x0, #MESON_STATUS_OFFSET]
	tbz	w1, #MESON_STATUS_TX_EMPTY_BIT, 1b
	mov	w0, #0
	ret
endfunc console_meson_core_flush
