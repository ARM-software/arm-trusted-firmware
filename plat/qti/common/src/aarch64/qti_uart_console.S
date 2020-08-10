/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020 The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <console_macros.S>

#include <platform_def.h>
#include <qti_uart_console.h>

/*
 * This driver implements console logging into a ring buffer.
 */

	.globl qti_console_uart_register

	/* -----------------------------------------------
	 * int qti_console_uart_register(console_t *console,
	 *				 uintptr_t uart_base_addr)
	 * Registers uart console instance.
	 * In:  x0 - pointer to empty console_t struct
	 *      x1 - start address of uart block.
	 * Out: x0 - 1 to indicate success
	 * Clobber list: x0, x1, x14
	 * -----------------------------------------------
	 */
func qti_console_uart_register
	str	x1, [x0, #CONSOLE_T_BASE]	/* Save UART base. */
	finish_console_register uart putc=1, flush=1
endfunc qti_console_uart_register

	/* -----------------------------------------------
	 * int qti_console_uart_puts(int c, console_t *console)
	 * Writes a character to the UART console.
	 * The character must be preserved in x0.
	 * In: x0 - character to be stored
	 *     x1 - pointer to console_t struct
	 * Clobber list: x1, x2
	 * -----------------------------------------------
	 */
func console_uart_putc
	/* set x1 = UART base. */
	ldr	x1, [x1, #CONSOLE_T_BASE]

	/* Loop until M_GENI_CMD_ACTIVE bit not clear. */
1:	ldr	w2, [x1, #GENI_STATUS_REG]
	and	w2, w2, #GENI_STATUS_M_GENI_CMD_ACTIVE_MASK
	cmp	w2, #GENI_STATUS_M_GENI_CMD_ACTIVE_MASK
	b.eq	1b

	/* Transmit data. */
	cmp	w0, #0xA
	b.ne	3f

	/* Add '\r' when input char is '\n' */
	mov	w2, #0x1
	mov	w0, #0xD
	str	w2, [x1, #UART_TX_TRANS_LEN_REG]
	mov	w2, #GENI_M_CMD_TX
	str	w2, [x1, #GENI_M_CMD0_REG]
	str	w0, [x1, #GENI_TX_FIFOn_REG]
	mov	w0, #0xA

	/* Loop until M_GENI_CMD_ACTIVE bit not clear. */
2:	ldr	w2, [x1, #GENI_STATUS_REG]
	and	w2, w2, #GENI_STATUS_M_GENI_CMD_ACTIVE_MASK
	cmp	w2, #GENI_STATUS_M_GENI_CMD_ACTIVE_MASK
	b.eq	2b

	/* Transmit i/p data. */
3:	mov	w2, #0x1
	str	w2, [x1, #UART_TX_TRANS_LEN_REG]
	mov	w2, #GENI_M_CMD_TX
	str	w2, [x1, #GENI_M_CMD0_REG]
	str	w0, [x1, #GENI_TX_FIFOn_REG]

	ret
endfunc	console_uart_putc

	/* -----------------------------------------------
	 * int qti_console_uart_flush(console_t *console)
	 * In:  x0 - pointer to console_t struct
	 * Out: x0 - 0 for success
	 * Clobber list: x0, x1
	 * -----------------------------------------------
	 */
func console_uart_flush
	/* set x0 = UART base. */
	ldr	x0, [x0, #CONSOLE_T_BASE]

	/* Loop until M_GENI_CMD_ACTIVE bit not clear. */
1:	ldr	w1, [x0, #GENI_STATUS_REG]
	and	w1, w1, #GENI_STATUS_M_GENI_CMD_ACTIVE_MASK
	cmp	w1, #GENI_STATUS_M_GENI_CMD_ACTIVE_MASK
	b.eq	1b

	mov	w0, #0
	ret
endfunc console_uart_flush
