/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <drivers/st/stm32_gpio.h>

#define GPIO_TX_SHIFT		(DEBUG_UART_TX_GPIO_PORT << 1)

	.globl	platform_mem_init
	.globl	plat_report_exception
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_reset_handler
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_crash_console_init
	.globl	plat_crash_console_flush
	.globl	plat_crash_console_putc
	.globl	plat_panic_handler

func platform_mem_init
	/* Nothing to do, don't need to init SYSRAM */
	bx	lr
endfunc platform_mem_init

func plat_report_exception
#if DEBUG
	mov	r8, lr

	/* Test if an abort occurred */
	cmp	r0, #MODE32_abt
	bne	undef_inst_lbl
	ldr	r4, =abort_str
	bl	asm_print_str
	mrs	r4, lr_abt
	sub	r4, r4, #4
	b	print_exception_info

undef_inst_lbl:
	/* Test for an undefined instruction */
	cmp	r0, #MODE32_und
	bne	other_exception_lbl
	ldr	r4, =undefined_str
	bl	asm_print_str
	mrs	r4, lr_und
	b	print_exception_info

other_exception_lbl:
	/* Other exceptions */
	mov	r9, r0
	ldr	r4, =exception_start_str
	bl	asm_print_str
	mov	r4, r9
	bl	asm_print_hex
	ldr	r4, =exception_end_str
	bl	asm_print_str
	mov	r4, r6

print_exception_info:
	bl	asm_print_hex

	ldr	r4, =end_error_str
	bl	asm_print_str

	bx	r8
#else
	bx	lr
#endif
endfunc plat_report_exception

func plat_reset_handler
	bx	lr
endfunc plat_reset_handler

	/* ------------------------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between a cold and warm
	 * boot.
	 *
	 * Currently supports only cold boot
	 * ------------------------------------------------------------------
	 */
func plat_get_my_entrypoint
	mov	r0, #0
	bx	lr
endfunc plat_get_my_entrypoint

	/* ---------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * Cold-booting secondary CPUs is not supported.
	 * ---------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	b	.
endfunc plat_secondary_cold_boot_setup

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary (void);
	 *
	 * Find out whether the current cpu is the primary cpu.
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	ldcopr	r0, MPIDR
	ldr	r1, =(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	and	r0, r1
	cmp	r0, #STM32MP_PRIMARY_CPU
	moveq	r0, #1
	movne	r0, #0
	bx	lr
endfunc plat_is_my_cpu_primary

	/* -------------------------------------------
	 *  int plat_stm32mp1_get_core_pos(int mpidr);
	 *
	 *  Return CorePos = (ClusterId * 4) + CoreId
	 * -------------------------------------------
	 */
func plat_stm32mp1_get_core_pos
	and	r1, r0, #MPIDR_CPU_MASK
	and	r0, r0, #MPIDR_CLUSTER_MASK
	add	r0, r1, r0, LSR #6
	bx	lr
endfunc plat_stm32mp1_get_core_pos

	/* ------------------------------------
	 *  unsigned int plat_my_core_pos(void)
	 * ------------------------------------
	 */
func plat_my_core_pos
	ldcopr	r0, MPIDR
	b	plat_stm32mp1_get_core_pos
endfunc plat_my_core_pos

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 *
	 * Initialize the crash console without a C Runtime stack.
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	/* Enable GPIOs for UART TX */
	ldr	r1, =(RCC_BASE + DEBUG_UART_TX_GPIO_BANK_CLK_REG)
	ldr	r2, [r1]
	/* Configure GPIO */
	orr	r2, r2, #DEBUG_UART_TX_GPIO_BANK_CLK_EN
	str	r2, [r1]
	ldr	r1, =DEBUG_UART_TX_GPIO_BANK_ADDRESS
	/* Set GPIO mode alternate */
	ldr	r2, [r1, #GPIO_MODE_OFFSET]
	bic	r2, r2, #(GPIO_MODE_MASK << GPIO_TX_SHIFT)
	orr	r2, r2, #(GPIO_MODE_ALTERNATE << GPIO_TX_SHIFT)
	str	r2, [r1, #GPIO_MODE_OFFSET]
	/* Set GPIO speed low */
	ldr	r2, [r1, #GPIO_SPEED_OFFSET]
	bic	r2, r2, #(GPIO_SPEED_MASK << GPIO_TX_SHIFT)
	str	r2, [r1, #GPIO_SPEED_OFFSET]
	/* Set no-pull */
	ldr	r2, [r1, #GPIO_PUPD_OFFSET]
	bic	r2, r2, #(GPIO_PULL_MASK << GPIO_TX_SHIFT)
	str	r2, [r1, #GPIO_PUPD_OFFSET]
	/* Set alternate */
#if DEBUG_UART_TX_GPIO_PORT >= GPIO_ALT_LOWER_LIMIT
	ldr	r2, [r1, #GPIO_AFRH_OFFSET]
	bic	r2, r2, #(GPIO_ALTERNATE_MASK << \
				((DEBUG_UART_TX_GPIO_PORT - GPIO_ALT_LOWER_LIMIT) << 2))
	orr	r2, r2, #(DEBUG_UART_TX_GPIO_ALTERNATE << \
				((DEBUG_UART_TX_GPIO_PORT - GPIO_ALT_LOWER_LIMIT) << 2))
	str	r2, [r1, #GPIO_AFRH_OFFSET]
#else
	ldr	r2, [r1, #GPIO_AFRL_OFFSET]
	bic	r2, r2, #(GPIO_ALTERNATE_MASK << (DEBUG_UART_TX_GPIO_PORT << 2))
	orr	r2, r2, #(DEBUG_UART_TX_GPIO_ALTERNATE << (DEBUG_UART_TX_GPIO_PORT << 2))
	str	r2, [r1, #GPIO_AFRL_OFFSET]
#endif
	/* Enable UART clock, with its source */
	ldr	r1, =(RCC_BASE + DEBUG_UART_TX_CLKSRC_REG)
	mov	r2, #DEBUG_UART_TX_CLKSRC
	str	r2, [r1]
	ldr	r1, =(RCC_BASE + DEBUG_UART_TX_EN_REG)
	ldr	r2, [r1]
	orr	r2, r2, #DEBUG_UART_TX_EN
	str	r2, [r1]

	ldr	r0, =STM32MP_DEBUG_USART_BASE
	ldr	r1, =STM32MP_DEBUG_USART_CLK_FRQ
	ldr	r2, =STM32MP_UART_BAUDRATE
	b	console_stm32_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * void plat_crash_console_flush(void)
	 *
	 * Flush the crash console without a C Runtime stack.
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	ldr	r0, =STM32MP_DEBUG_USART_BASE
	b	console_stm32_core_flush
endfunc plat_crash_console_flush

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 *
	 * Print a character on the crash console without a C Runtime stack.
	 * Clobber list : r1 - r3
	 *
	 * In case of bootloading through uart, we keep console crash as this.
	 * Characters could be sent to the programmer, but will be ignored.
	 * No specific code in that case.
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	ldr	r1, =STM32MP_DEBUG_USART_BASE
	b	console_stm32_core_putc
endfunc plat_crash_console_putc

	/* ----------------------------------------------------------
	 * void plat_panic_handler(void) __dead2;
	 * Report exception + endless loop.
	 *
	 * r6 holds the address where the fault occurred.
	 * Filling lr with this value allows debuggers to reconstruct
	 * the backtrace.
	 * ----------------------------------------------------------
	 */
func plat_panic_handler
	mrs	r0, cpsr
	and	r0, #MODE32_MASK
	bl	plat_report_exception
	mov	lr, r6
	b	.
endfunc plat_panic_handler

#if DEBUG
.section .rodata.rev_err_str, "aS"
abort_str:
	.asciz "\nAbort at: 0x"
undefined_str:
	.asciz "\nUndefined instruction at: 0x"
exception_start_str:
	.asciz "\nException mode=0x"
exception_end_str:
	.asciz " at: 0x"
end_error_str:
	.asciz "\n\r"
#endif
