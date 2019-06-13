/*
 * Copyright (c) Linaro 2018-2019 Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <platform_def.h>
#include <imx_hab.h>

	.globl	platform_mem_init
	.globl	plat_get_my_entrypoint
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_panic_handler

	/* ---------------------------------------------
	 * int plat_mem_init(void)
	 * Function to initialize memory.
	 * The HAB hands off the DDR controller already
	 * setup and ready to use.
	 * Implement the mandatory function as a NOP
	 * ---------------------------------------------
	 */
func platform_mem_init
	bx	lr
endfunc platform_mem_init

func plat_get_my_entrypoint
	mov	r0, #0
	bx	lr
endfunc plat_get_my_entrypoint

func plat_crash_console_init
	mov_imm	r0, PLAT_IMX7_BOOT_UART_BASE
	mov_imm	r1, PLAT_IMX7_BOOT_UART_CLK_IN_HZ
	mov_imm	r2, PLAT_IMX7_CONSOLE_BAUDRATE
	b	imx_crash_uart_init
endfunc plat_crash_console_init

func plat_crash_console_putc
	mov_imm r1, PLAT_IMX7_BOOT_UART_BASE
	b	imx_crash_uart_putc
endfunc plat_crash_console_putc

func plat_crash_console_flush
	/* Placeholder */
	mov	r0, #0
	bx	lr
endfunc plat_crash_console_flush

func plat_panic_handler
	mov	r3, #HAB_ROM_VECTOR_TABLE_FAILSAFE
	ldr	r3, [r3, #0]
	blx	r3
endfunc plat_panic_handler
