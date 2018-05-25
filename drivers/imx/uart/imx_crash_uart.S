/*
 * Copyright (c) Linaro 2018 Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <imx_uart.h>
#include <platform_def.h>

	.globl	imx_crash_uart_init
	.globl	imx_crash_uart_putc

	/* -----------------------------------------------
	 * int imx_crash_uart_init(uintptr_t base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: r0 - console base address
	 *     r1 - Uart clock in Hz
	 *     r2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : r1, r2, r3, r4
	 * -----------------------------------------------
	 */
func imx_crash_uart_init
	/* Free up r1 as a scratch reg */
	mov     r4, r0
	mov     r0, r1

	/* Reset UART via CR2 */
	add     r1, r4, #IMX_UART_CR2_OFFSET
	movs    r3, #0
	str     r3, [r4, #IMX_UART_CR2_OFFSET]

	/* Wait for reset complete */
__wait_cr2_reset:
	ldr     r3, [r1, #0]
	ands    r3, #IMX_UART_CR2_SRST
	beq     __wait_cr2_reset

	/* Enable UART */
	movs    r3, #IMX_UART_CR1_UARTEN
	mov     r1, r2
	str     r3, [r4, #IMX_UART_CR1_OFFSET]

	/*
	 * Ignore RTC/CTS - disable reset
	 * Magic value #16423 =>
	 * IMX_UART_CR2_IRTS | IMX_UART_CR2_WS | IMX_UART_CR2_TXEN | IMX_UART_CR2_RXEN | IMX_UART_CR2_SRST
	 */
	movw    r3, #16423
	str     r3, [r4, #IMX_UART_CR2_OFFSET]

	/*
	 * No parity, autobaud detect-old, rxdmuxsel=1 (fixed i.mx7)
	 * Magic value => #132
	 * IMX_UART_CR3_ADNIMP | IMX_UART_CR3_RXDMUXSEL
	 */
	movs    r3, #132
	str     r3, [r4, #IMX_UART_CR3_OFFSET]

	/*
	 * Set CTS FIFO trigger to 32 bytes bits 15:10
	 * Magic value => #32768
	 * FIFO trigger bitmask 100000
	 * */
	mov     r3, #32768
	str     r3, [r4, #IMX_UART_CR4_OFFSET]

	/*
	 * TX/RX-thresh = 2 bytes, DCE (bit6 = 0), refclk @24MHz / 4
	 * Magic value #2562
	 * IMX_UART_FCR_TXTL(TX_RX_THRESH) | IMX_UART_FCR_RXTL(TX_RX_THRESH) | IMX_UART_FCR_RFDIV2
	 */
	#ifdef IMX_UART_DTE
	movw    r3, #2626
	#else
	movw    r3, #2562
	#endif
	str     r3, [r4, #IMX_UART_FCR_OFFSET]

	/* This BIR should be set to 0x0F prior to writing the BMR */
	movs    r3, #15
	str     r3, [r4, #IMX_UART_BIR_OFFSET]

	/* Hard-code to 115200 @ 24 MHz */
	movs	r0, #104
	str     r0, [r4, #IMX_UART_BMR_OFFSET]

	/* Indicate success */
	movs    r0, #1
	bx	lr
endfunc imx_crash_uart_init

	/* --------------------------------------------------------
	 * int imx_crash_uart_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : r0 - character to be printed
	 *      r1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : r2
	 * --------------------------------------------------------
	 */
func imx_crash_uart_putc
	/* Output specified character to UART shift-register */
	str	r0, [r1, #IMX_UART_TXD_OFFSET]

        /* Wait for transmit IMX_UART_STAT2_OFFSET.IMX_UART_STAT2_TXDC == 1 */
__putc_spin_ready:
	ldr	r2, [r1, #IMX_UART_STAT2_OFFSET]
	ands	r2, #IMX_UART_STAT2_TXDC
	beq	__putc_spin_ready

        /* Transmit complete do we need to fixup \n to \n\r */
	cmp	r0, #10
	beq	__putc_fixup_lf

	/* No fixup necessary - exit here */
	movs	r0, #0
	bx	lr

	/* Fixup \n to \n\r */
__putc_fixup_lf:
	movs	r0, #13
	b	imx_crash_uart_putc
endfunc imx_crash_uart_putc
