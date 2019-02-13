/*
 * Copyright (c) Linaro 2018 Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <platform_def.h>

#include <arch.h>
#include <lib/mmio.h>

#include <imx_uart.h>

/* TX/RX FIFO threshold */
#define TX_RX_THRESH 2

struct clk_div_factors {
	uint32_t fcr_div;
	uint32_t bmr_div;
};

static struct clk_div_factors clk_div[] = {
	{
		.fcr_div = IMX_UART_FCR_RFDIV1,
		.bmr_div = 1,
	},
	{
		.fcr_div = IMX_UART_FCR_RFDIV2,
		.bmr_div = 2,
	},
	{
		.fcr_div = IMX_UART_FCR_RFDIV3,
		.bmr_div = 3,
	},
	{
		.fcr_div = IMX_UART_FCR_RFDIV4,
		.bmr_div = 4,
	},
	{
		.fcr_div = IMX_UART_FCR_RFDIV5,
		.bmr_div = 5,
	},
	{
		.fcr_div = IMX_UART_FCR_RFDIV6,
		.bmr_div = 6,
	},
	{
		.fcr_div = IMX_UART_FCR_RFDIV7,
		.bmr_div = 7,
	},
};

static void write_reg(uintptr_t base, uint32_t offset, uint32_t val)
{
	mmio_write_32(base + offset, val);
}

static uint32_t read_reg(uintptr_t base, uint32_t offset)
{
	return mmio_read_32(base + offset);
}

int console_imx_uart_core_init(uintptr_t base_addr, unsigned int uart_clk,
			       unsigned int baud_rate)
{
	uint32_t val;
	uint8_t clk_idx = 1;

	/* Reset UART */
	write_reg(base_addr, IMX_UART_CR2_OFFSET, 0);
	do {
		val = read_reg(base_addr, IMX_UART_CR2_OFFSET);
	} while (!(val & IMX_UART_CR2_SRST));

	/* Enable UART */
	write_reg(base_addr, IMX_UART_CR1_OFFSET, IMX_UART_CR1_UARTEN);

	/* Ignore RTS, 8N1, enable tx/rx, disable reset */
	val = (IMX_UART_CR2_IRTS | IMX_UART_CR2_WS | IMX_UART_CR2_TXEN |
	       IMX_UART_CR2_RXEN | IMX_UART_CR2_SRST);
	write_reg(base_addr, IMX_UART_CR2_OFFSET, val);

	/* No parity, autobaud detect-old, rxdmuxsel=1 (fixed i.mx7) */
	val = IMX_UART_CR3_ADNIMP | IMX_UART_CR3_RXDMUXSEL;
	write_reg(base_addr, IMX_UART_CR3_OFFSET, val);

	/* Set CTS FIFO trigger to 32 bytes bits 15:10 */
	write_reg(base_addr, IMX_UART_CR4_OFFSET, 0x8000);

	/* TX/RX-thresh = 2 bytes, DTE (bit6 = 0), refclk @24MHz / 4 */
	val = IMX_UART_FCR_TXTL(TX_RX_THRESH) | IMX_UART_FCR_RXTL(TX_RX_THRESH) |
	      clk_div[clk_idx].fcr_div;
	#ifdef IMX_UART_DTE
		/* Set DTE (bit6 = 1) */
		val |= IMX_UART_FCR_DCEDTE;
	#endif
	write_reg(base_addr, IMX_UART_FCR_OFFSET, val);

	/*
	 * The equation for BAUD rate calculation is
	 * RefClk = Supplied clock / FCR_DIVx
	 *
	 * BAUD  =    Refclk
	 *         ------------
	 *       16 x (UBMR + 1/ UBIR + 1)
	 *
	 * We write 0x0f into UBIR to remove the 16 mult
	 * BAUD  =    6000000
	 *         ------------
	 *       16 x (UBMR + 1/ 15 + 1)
	 */

	write_reg(base_addr, IMX_UART_BIR_OFFSET, 0x0f);
	val = ((uart_clk / clk_div[clk_idx].bmr_div) / baud_rate) - 1;
	write_reg(base_addr, IMX_UART_BMR_OFFSET, val);

	return 0;
}

/* --------------------------------------------------------
 * int console_core_putc(int c, uintptr_t base_addr)
 * Function to output a character over the console. It
 * returns the character printed on success or -1 on error.
 * In : r0 - character to be printed
 *      r1 - console base address
 * Out : return -1 on error else return character.
 * Clobber list : r2
 * --------------------------------------------------------
 */
int console_imx_uart_core_putc(int c, uintptr_t base_addr)
{
	uint32_t val;

	if (c == '\n')
		console_imx_uart_core_putc('\r', base_addr);

	/* Write data */
	write_reg(base_addr, IMX_UART_TXD_OFFSET, c);

	/* Wait for transmit */
	do {
		val = read_reg(base_addr, IMX_UART_STAT2_OFFSET);
	} while (!(val & IMX_UART_STAT2_TXDC));

	return 0;
}

/*
 * Function to get a character from the console.
 * It returns the character grabbed on success
 * or -1 on error.
 * In : r0 - console base address
 * Clobber list : r0, r1
 * ---------------------------------------------
 */
int console_imx_uart_core_getc(uintptr_t base_addr)
{
	uint32_t val;

	val = read_reg(base_addr, IMX_UART_TS_OFFSET);
	if (val & IMX_UART_TS_RXEMPTY)
		return -1;

	val = read_reg(base_addr, IMX_UART_RXD_OFFSET);
	return (int)(val & 0x000000FF);
}

/*
 * Function to force a write of all buffered
 * data that hasn't been output.
 * In : r0 - console base address
 * Out : return -1 on error else return 0.
 * Clobber list : r0, r1
 * ---------------------------------------------
 */
int console_imx_uart_core_flush(uintptr_t base_addr)
{
	return 0;
}

