/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <console_macros.S>
#include <assert_macros.S>
#include "imx_uart.h"

#define URXD  0x0  /* Receiver Register */
#define UTXD  0x40 /* Transmitter Register */
#define UTS   0xb4 /* UART Test Register (mx31) */
#define  URXD_RX_DATA    (0xFF)

	.globl	console_imx_uart_register
	.globl	console_imx_uart_init
	.globl	console_imx_uart_putc
	.globl	console_imx_uart_getc
	.globl	console_imx_uart_flush

func console_imx_uart_register
	mov	x7, x30
	mov	x6, x3
	cbz	x6, register_fail
	str	x0, [x6, #CONSOLE_T_BASE]

	bl	console_imx_uart_init
	cbz	x0, register_fail

	mov	x0, x6
	mov	x30, x7
	finish_console_register imx_uart putc=1, getc=1, flush=1

register_fail:
	ret	x7
endfunc console_imx_uart_register

func console_imx_uart_init
	mov	w0, #1
	ret
endfunc console_imx_uart_init

func console_imx_uart_putc
	ldr	x1, [x1, #CONSOLE_T_BASE]
	cbz	x1, putc_error

	/* Prepare '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
1:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #UTS]
	tbz	w2, #6, 1b
	mov	w2, #0xD
	str	w2, [x1, #UTXD]
2:
	/* Check if the transmit FIFO is full */
	ldr	w2, [x1, #UTS]
	tbz	w2, #6, 2b
	str	w0, [x1, #UTXD]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_imx_uart_putc

func console_imx_uart_getc
	ldr	x0, [x0, #CONSOLE_T_BASE]
	cbz	x0, getc_error
1:
	ldr	w1, [x0, #UTS]
	tbnz	w1, #5, 1b

	ldr	w1, [x0, #URXD]
	and	w0, w1, #URXD_RX_DATA

	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_imx_uart_getc

func console_imx_uart_flush
	ret
endfunc console_imx_uart_flush
