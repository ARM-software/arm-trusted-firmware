/*
 * Copyright (c) 2023, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <drivers/st/stm32_gpio.h>

#include <platform_def.h>

#define GPIO_TX_SHIFT		(DEBUG_UART_TX_GPIO_PORT << 1)

	.globl	platform_mem_init
	.globl	plat_secondary_cold_boot_setup
	.globl	plat_is_my_cpu_primary
	.globl	plat_crash_console_init
	.globl	plat_crash_console_flush
	.globl	plat_crash_console_putc
	.globl	plat_report_exception

func platform_mem_init
	/* Nothing to do, don't need to init SYSRAM */
	ret
endfunc platform_mem_init

	/* ---------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * Set secondary core in WFI waiting for core reset.
	 * ---------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	dsb	sy
	wfi
	/* This shouldn't be reached */
	b	.
endfunc plat_secondary_cold_boot_setup

	/* ----------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 * This function checks if this is the primary CPU
	 * ----------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CPU_MASK)
	cmp	x0, #STM32MP_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 *
	 * Initialize the crash console without a C Runtime stack.
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	/* Reset UART peripheral */
	mov_imm	x1, (RCC_BASE + DEBUG_UART_RST_REG)
	ldr	x2, =DEBUG_UART_RST_BIT
	ldr	x0, [x1]
	orr	x0, x0, x2
	str	x0, [x1]
1:
	ldr	x0, [x1]
	ands	x2, x0, x2
	beq	1b
	bic	x2, x2, #DEBUG_UART_RST_BIT
	str	x2, [x1]
2:
	ldr	x0, [x1]
	ands	x2, x0, x2
	bne	2b
	/* Enable GPIOs for UART TX */
	mov_imm	x1, (RCC_BASE + DEBUG_UART_TX_GPIO_BANK_CLK_REG)
	ldr	w2, [x1]
	/* Configure GPIO */
	orr	w2, w2, #DEBUG_UART_TX_GPIO_BANK_CLK_EN
	str	w2, [x1]
	mov_imm	x1, DEBUG_UART_TX_GPIO_BANK_ADDRESS
	/* Set GPIO mode alternate */
	ldr	w2, [x1, #GPIO_MODE_OFFSET]
	bic	w2, w2, #(GPIO_MODE_MASK << GPIO_TX_SHIFT)
	orr	w2, w2, #(GPIO_MODE_ALTERNATE << GPIO_TX_SHIFT)
	str	w2, [x1, #GPIO_MODE_OFFSET]
	/* Set GPIO speed low */
	ldr	w2, [x1, #GPIO_SPEED_OFFSET]
	bic	w2, w2, #(GPIO_SPEED_MASK << GPIO_TX_SHIFT)
	str	w2, [x1, #GPIO_SPEED_OFFSET]
	/* Set no-pull */
	ldr	w2, [x1, #GPIO_PUPD_OFFSET]
	bic	w2, w2, #(GPIO_PULL_MASK << GPIO_TX_SHIFT)
	str	w2, [x1, #GPIO_PUPD_OFFSET]
	/* Set alternate */
#if DEBUG_UART_TX_GPIO_PORT >= GPIO_ALT_LOWER_LIMIT
	ldr	w2, [x1, #GPIO_AFRH_OFFSET]
	bic	w2, w2, #(GPIO_ALTERNATE_MASK << \
				((DEBUG_UART_TX_GPIO_PORT - GPIO_ALT_LOWER_LIMIT) << 2))
	orr	w2, w2, #(DEBUG_UART_TX_GPIO_ALTERNATE << \
				((DEBUG_UART_TX_GPIO_PORT - GPIO_ALT_LOWER_LIMIT) << 2))
	str	w2, [x1, #GPIO_AFRH_OFFSET]
#else
	ldr	w2, [x1, #GPIO_AFRL_OFFSET]
	bic	w2, w2, #(GPIO_ALTERNATE_MASK << (DEBUG_UART_TX_GPIO_PORT << 2))
	orr	w2, w2, #(DEBUG_UART_TX_GPIO_ALTERNATE << (DEBUG_UART_TX_GPIO_PORT << 2))
	str	w2, [x1, #GPIO_AFRL_OFFSET]
#endif
	/* Clear UART clock flexgen divisors, keep enable bit */
	mov_imm	x1, (RCC_BASE + DEBUG_UART_PREDIV_CFGR)
	mov	x2, #0
	str	w2, [x1]
	mov_imm	x1, (RCC_BASE + DEBUG_UART_FINDIV_CFGR)
	mov	x2, #0x40
	str	w2, [x1]
	/* Enable UART clock, with its source */
	mov_imm	x1, (RCC_BASE + DEBUG_UART_TX_CLKSRC_REG)
	mov_imm	w2, (DEBUG_UART_TX_CLKSRC | RCC_XBARxCFGR_XBARxEN)
	str	w2, [x1]
	mov_imm	x1, (RCC_BASE + DEBUG_UART_TX_EN_REG)
	ldr	w2, [x1]
	orr	w2, w2, #DEBUG_UART_TX_EN
	str	w2, [x1]

	mov_imm	x0, STM32MP_DEBUG_USART_BASE
	mov_imm	x1, STM32MP_DEBUG_USART_CLK_FRQ
	mov_imm	x2, STM32MP_UART_BAUDRATE
	b	console_stm32_core_init
endfunc plat_crash_console_init

func plat_crash_console_flush
	mov_imm	x0, STM32MP_DEBUG_USART_BASE
	b	console_stm32_core_flush
endfunc plat_crash_console_flush

func plat_crash_console_putc
	mov_imm	x1, STM32MP_DEBUG_USART_BASE
	cmp	x0, #'\n'
	b.ne	1f
	mov	x15, x30
	mov	x0, #'\r'
	bl	console_stm32_core_putc
	mov	x30, x15
	mov	x0, #'\n'
1:
	b	console_stm32_core_putc
endfunc plat_crash_console_putc

#ifdef IMAGE_BL2
	/* ---------------------------------------------
	 * void plat_report_exception(unsigned int type)
	 * Function to report an unhandled exception
	 * with platform-specific means.
	 * ---------------------------------------------
	 */
func plat_report_exception
	mov	x8, x30

	adr	x4, plat_err_str
	bl	asm_print_str

	adr	x4, esr_el3_str
	bl	asm_print_str

	mrs	x4, esr_el3
	bl	asm_print_hex

	adr	x4, elr_el3_str
	bl	asm_print_str

	mrs	x4, elr_el3
	bl	asm_print_hex

	adr	x4, far_el3_str
	bl	asm_print_str

	mrs	x4, far_el3
	bl	asm_print_hex

	mov	x30, x8
	ret
endfunc plat_report_exception

.section .rodata.rev_err_str, "aS"
plat_err_str:
	.asciz "\nPlatform exception reporting:"
esr_el3_str:
	.asciz "\nESR_EL3: "
elr_el3_str:
	.asciz "\nELR_EL3: "
far_el3_str:
	.asciz "\nFAR_EL3: "
#endif /* IMAGE_BL2 */
