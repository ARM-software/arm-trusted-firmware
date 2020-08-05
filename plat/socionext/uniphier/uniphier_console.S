/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <drivers/console.h>

#include "uniphier_console.h"

/*
 * In: w0 - character to be printed
 *     x1 - pointer to console structure
 * Out: return the character written (always succeeds)
 * Clobber: x2
 */
	.globl	uniphier_console_putc
func uniphier_console_putc
	ldr	x1, [x1, #CONSOLE_T_BASE]

	/* Wait until the transmitter FIFO gets empty */
0:	ldr	w2, [x1, #UNIPHIER_UART_LSR]
	tbz	w2, #UNIPHIER_UART_LSR_THRE_BIT, 0b

	str	w0, [x1, #UNIPHIER_UART_TX]

	ret
endfunc uniphier_console_putc

/*
 * In: x0 - pointer to console structure
 * Out: return the character read, or ERROR_NO_PENDING_CHAR if no character
	is available
 * Clobber: x1
 */
	.globl	uniphier_console_getc
func uniphier_console_getc
	ldr	x0, [x0, #CONSOLE_T_BASE]

	ldr	w1, [x0, #UNIPHIER_UART_LSR]
	tbz	w1, #UNIPHIER_UART_LSR_DR_BIT, 0f

	ldr	w0, [x0, #UNIPHIER_UART_RX]
	ret

0:	mov	w0, #ERROR_NO_PENDING_CHAR
	ret
endfunc uniphier_console_getc

/*
 * In: x0 - pointer to console structure
 * Out: return 0 (always succeeds)
 * Clobber: x1
 */
	.global uniphier_console_flush
func uniphier_console_flush
	ldr	x0, [x0, #CONSOLE_T_BASE]

	/* wait until the transmitter gets empty */
0:	ldr	w1, [x0, #UNIPHIER_UART_LSR]
	tbz	w1, #UNIPHIER_UART_LSR_TEMT_BIT, 0b

	ret
endfunc uniphier_console_flush
