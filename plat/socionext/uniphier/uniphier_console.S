/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

#define UNIPHIER_UART_BASE	0x54006800
#define UNIPHIER_UART_END	0x54006c00
#define UNIPHIER_UART_OFFSET	0x100

#define UNIPHIER_UART_RX	0x00	/* In:  Receive buffer */
#define UNIPHIER_UART_TX	0x00	/* Out: Transmit buffer */

#define UNIPHIER_UART_FCR	0x0c	/* Char/FIFO Control Register */
#define   UNIPHIER_UART_FCR_ENABLE_FIFO	0x01	/* Enable the FIFO */

#define UNIPHIER_UART_LCR_MCR	0x10	/* Line/Modem Control Register */
#define   UNIPHIER_UART_LCR_WLEN8	0x03	/* Wordlength: 8 bits */
#define UNIPHIER_UART_LSR	0x14	/* Line Status Register */
#define   UNIPHIER_UART_LSR_TEMT_BIT	6	/* Transmitter empty */
#define   UNIPHIER_UART_LSR_THRE_BIT	5	/* Transmit-hold-register empty */
#define   UNIPHIER_UART_LSR_DR_BIT	0	/* Receiver data ready */
#define UNIPHIER_UART_DLR	0x24	/* Divisor Latch Register */

/*
 * Uncomment for debug
 */
/* #define UNIPHIER_UART_INIT_DIVISOR */
#define UNIPHIER_UART_DEFAULT_BASE	(UNIPHIER_UART_BASE)
#define UNIPHIER_UART_CLK_RATE		58820000
#define UNIPHIER_UART_DEFAULT_BAUDRATE	115200

/*
 * In: x0 - console base address
 *     w1 - uart clock in Hz
 *     w2 - baud rate
 * Out: return 1 on success, or 0 on error
 */
	.globl	console_core_init
func console_core_init
	cbz	x0, 1f
#ifdef UNIPHIER_UART_INIT_DIVISOR
	cbz	w1, 1f
	cbz	w2, 1f
	/* divisor = uart_clock / (16 * baud_rate) */
	udiv	w2, w1, w2
	lsr	w2, w2, #4
#endif
	/* Make sure the transmitter is empty before the divisor set/change */
0:	ldr	w1, [x0, #UNIPHIER_UART_LSR]
	tbz	w1, #UNIPHIER_UART_LSR_TEMT_BIT, 0b
#ifdef UNIPHIER_UART_INIT_DIVISOR
	str	w2, [x0, #UNIPHIER_UART_DLR]
#endif
	mov	w2, #UNIPHIER_UART_FCR_ENABLE_FIFO
	str	w2, [x0, #UNIPHIER_UART_FCR]

	mov	w2, #(UNIPHIER_UART_LCR_WLEN8 << 8)
	str	w2, [x0, #UNIPHIER_UART_LCR_MCR]

	mov	w0, #1
	ret
1:	mov	w0, #0
	ret
endfunc console_core_init

/*
 * In: w0 - character to be printed
 *     x1 - console base address
 * Out: return the character written, or -1 on error
 * Clobber: x2
 */
	.globl	console_core_putc
func console_core_putc
	/* Error out if the console is not initialized */
	cbz	x1, 2f

	/* Wait until the transmitter FIFO gets empty */
0:	ldr	w2, [x1, #UNIPHIER_UART_LSR]
	tbz	w2, #UNIPHIER_UART_LSR_THRE_BIT, 0b

	mov	w2, w0

1:	str	w2, [x1, #UNIPHIER_UART_TX]

	cmp	w2, #'\n'
	b.ne	3f
	mov	w2, #'\r'	/* Append '\r' to '\n' */
	b	1b
2:	mov	w0, #-1
3:	ret
endfunc console_core_putc

/*
 * In: x0 - console base address
 * Out: return the character read
 * Clobber: x1
 */
	.globl	console_core_getc
func console_core_getc
	/* Error out if the console is not initialized */
	cbz	x0, 1f

	/* Wait while the receiver FIFO is empty */
0:	ldr	w1, [x0, #UNIPHIER_UART_LSR]
	tbz	w1, #UNIPHIER_UART_LSR_DR_BIT, 0b

	ldr	w0, [x0, #UNIPHIER_UART_RX]

	ret
1:	mov	w0, #-1
	ret
endfunc console_core_getc

/*
 * In:  x0 - console base address
 * Out: return 0, or -1 on error
 * Clobber: x1
 */
	.global console_core_flush
func console_core_flush
	/* Error out if the console is not initialized */
	cbz	x0, 1f

	/* wait until the transmitter gets empty */
0:	ldr	w1, [x0, #UNIPHIER_UART_LSR]
	tbz	w1, #UNIPHIER_UART_LSR_TEMT_BIT, 0b

	mov	w0, #0
	ret
1:	mov	w0, #-1
	ret
endfunc console_core_flush

/* find initialized UART port */
.macro uniphier_console_get_base base, tmpx, tmpw
	ldr	\base, =UNIPHIER_UART_BASE
0000:	ldr	\tmpw, [\base, #UNIPHIER_UART_DLR]
	mvn	\tmpw, \tmpw
	uxth	\tmpw, \tmpw
	cbnz	\tmpw, 0001f
	add	\base, \base, #UNIPHIER_UART_OFFSET
	ldr	\tmpx, =UNIPHIER_UART_END
	cmp	\base, \tmpx
	b.lo	0000b
	mov	\base, #0
0001:
.endm

/*
 * int plat_crash_console_init(void)
 * Clobber: x0-x2
 */
	.globl	plat_crash_console_init
func plat_crash_console_init
#ifdef UNIPHIER_UART_INIT_DIVISOR
	ldr	x0, =UNIPHIER_UART_DEFAULT_BASE
	ldr	x1, =UNIPHIER_UART_CLK_RATE
	ldr	x2, =UNIPHIER_UART_DEFAULT_BAUDRATE
	b	console_core_init
#else
	ret
#endif
endfunc plat_crash_console_init

/*
 * int plat_crash_console_putc(int c)
 * Clobber: x1, x2
 */
	.globl	plat_crash_console_putc
func plat_crash_console_putc
#ifdef UNIPHIER_UART_INIT_DIVISOR
	ldr	x1, =UNIPHIER_UART_DEFAULT_BASE
#else
	uniphier_console_get_base x1, x2, w2
#endif
	b	console_core_putc
endfunc plat_crash_console_putc

/*
 * int plat_crash_console_flush(void)
 * Clobber: x0, x1
 */
	.global plat_crash_console_flush
func plat_crash_console_flush
#ifdef UNIPHIER_UART_INIT_DIVISOR
	ldr	x0, =UNIPHIER_UART_DEFAULT_BASE
#else
	uniphier_console_get_base x0, x1, w1
#endif
	b	console_core_flush
endfunc plat_crash_console_flush

/*
 * void uniphier_console_setup(void)
 * Clobber: x0-x2
 */
	.globl	uniphier_console_setup
func uniphier_console_setup
#ifdef UNIPHIER_UART_INIT_DIVISOR
	ldr	x0, =UNIPHIER_UART_DEFAULT_BASE
	ldr	w1, =UNIPHIER_UART_CLK_RATE
	ldr	w2, =UNIPHIER_UART_DEFAULT_BAUDRATE
#else
	uniphier_console_get_base x0, x1, w1
	mov	w1, #0
	mov	w2, #0
#endif
	b	console_init
endfunc uniphier_console_setup
