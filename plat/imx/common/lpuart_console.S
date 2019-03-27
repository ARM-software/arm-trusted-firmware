/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <console_macros.S>
#include <assert_macros.S>
#include "imx8_lpuart.h"

	.globl	console_lpuart_register
	.globl	console_lpuart_init
	.globl	console_lpuart_putc
	.globl	console_lpuart_getc
	.globl	console_lpuart_flush

func console_lpuart_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_DRVDATA]

	bl	console_lpuart_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register lpuart putc=1, getc=1, flush=1

register_fail:
	ret	x7
endfunc console_lpuart_register

func console_lpuart_init
	mov	w0, #1
	ret
endfunc console_lpuart_init

func console_lpuart_putc
	ldr	x1, [x1, #CONSOLE_T_DRVDATA]
	cbz	x1, putc_error
	/* Prepare '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
1:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #STAT]
	tbz	w2, #23, 1b
	mov	w2, #0xD
	str	w2, [x1, #DATA]
2:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #STAT]
	tbz	w2, #23, 2b
	str	w0, [x1, #DATA]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_lpuart_putc

func console_lpuart_getc
	ldr	x0, [x0, #CONSOLE_T_DRVDATA]
	cbz	x0, getc_error
	/* Check if the receive FIFO state */
	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_lpuart_getc

func console_lpuart_flush
	mov	x0, #0
	ret
endfunc console_lpuart_flush
