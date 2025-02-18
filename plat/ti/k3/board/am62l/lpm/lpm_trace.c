/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <k3_lpm_timeout.h>
#include <lpm_trace.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#define UART_16550_THR			0x00U
#define UART_16550_LSR			0x14U
/* Line Status Register bits */
#define UART_16550_LSR_SR_E		BIT(6)
#define UART_16550_LSR_TX_FIFO_E	BIT(5)
#define TRACE_HEXADECIMAL_BASE		16U
/* TODO: Recheck the timeout value */
#define K3_UART_TIMEOUT_TX		10000U

/* TODO: Find a better way to enable the UART driver */
/**
 * \brief Convert integer value to hexadecimal string and store output from least significant
 *		   nibble to most significant nibble
 * \param value Input integer value to convert
 * \param str   Pointer to location where output hexadecimal string is stored
 */
__wkupsramfunc static uint8_t  lpm_trace_int_to_hex(uint32_t value, uint8_t *str)
{
	uint32_t val_rem;
	uint8_t idx = 0U;

	if (value == 0U) {
		str[idx] = (uint8_t)'0';
		idx++;
	} else {
		while (value > 0U) {
			val_rem = value % TRACE_HEXADECIMAL_BASE;
			if (val_rem < 10U) {
				str[idx] = (uint8_t)(val_rem + (uint8_t)'0');
			} else {
				str[idx] = (uint8_t)((val_rem - 10U) + (uint8_t)'A');
			}
			value /= TRACE_HEXADECIMAL_BASE;
			idx++;
		}
	}

	str[idx] = (uint8_t)'\0';

	if (idx > 1U) {
		/* Get length of string - NULL terminator */
		idx--;
	}

	return idx;
}

/**
 * \brief Put a uint8_t character into the UART register
 * \param data character to write
 */
__wkupsramfunc static void lpm_console_tx(uint8_t data)
{
	uint32_t val;
	uint32_t i = 0U;

	/*
	 * Poll the Line Status Register to ensure FIFO space is
	 * available before writing to avoid dropping chars.
	 */
	do {
		val = mmio_read_32(K3_WKUP_UART_BASE_ADDRESS + UART_16550_LSR);
		k3_lpm_delay_1us();
	} while ((i++ < K3_UART_TIMEOUT_TX) && ((val & UART_16550_LSR_TX_FIFO_E) == 0U));

	mmio_write_32((K3_WKUP_UART_BASE_ADDRESS + UART_16550_THR), data);

}

__wkupsramfunc static void lpm_trace_debug_uart(uint8_t *str, uint8_t len)
{
	uint32_t i;

	/* Output "0x" at start of hex */
	lpm_console_tx('0');
	lpm_console_tx('x');

	/*
	 * Output string backwards as we converted from low
	 * digit to high
	 */
	for (i = 0U; i <= len; i++) {
		lpm_console_tx(str[len - i]);
	}

	/* Add a carriage return to support unflexible terminals. */
	lpm_console_tx('\r');

	/* Move the cursor to new line. */
	lpm_console_tx('\n');
}

__wkupsramfunc void lpm_trace_debug(uint32_t value)
{
	uint8_t str[9];
	uint8_t len;

	len = lpm_trace_int_to_hex(value, str);
	lpm_trace_debug_uart(str, len);
}

