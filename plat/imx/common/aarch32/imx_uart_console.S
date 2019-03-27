/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <console_macros.S>
#include <assert_macros.S>
#include "imx_uart.h"

	.globl	console_imx_uart_register
	.globl	console_imx_uart_putc
	.globl	console_imx_uart_getc
	.globl	console_imx_uart_flush

func console_imx_uart_register
	push	{r4, lr}
	mov	r4, r3
	cmp	r4, #0
	beq	register_fail
	str	r0, [r4, #CONSOLE_T_DRVDATA]

	bl	console_imx_uart_core_init
	cmp	r0, #0
	bne	register_fail

	mov	r0, r4
	pop	{r4, lr}
	finish_console_register imx_uart putc=1, getc=1, flush=1

register_fail:
	pop	{r4, pc}
endfunc console_imx_uart_register

func console_imx_uart_putc
	ldr	r1, [r1, #CONSOLE_T_DRVDATA]
	b console_imx_uart_core_putc
endfunc console_imx_uart_putc

func console_imx_uart_getc
	ldr	r0, [r0, #CONSOLE_T_DRVDATA]
	b console_imx_uart_core_getc
endfunc console_imx_uart_getc

func console_imx_uart_flush
	ldr	r0, [r0, #CONSOLE_T_DRVDATA]
	b console_imx_uart_core_flush
endfunc console_imx_uart_flush
