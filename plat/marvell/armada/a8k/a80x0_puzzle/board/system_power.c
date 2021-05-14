/*
 * Copyright (C) 2020 Sartura Ltd.
 * Author: Luka Kovacic <luka.kovacic@sartura.hr>
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <armada_common.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/ti/uart/uart_16550.h>
#include <drivers/console.h>
#include <plat_marvell.h>

/*****************************************************************************
 * Platform specific power off functions
 * Power off PSU / Send command to power management MCU / ...
 *****************************************************************************
 */

unsigned char add_xor_checksum(unsigned char *buf, unsigned char xor_len)
{
	unsigned char xor_sum = 0;
	unsigned int i;

	for (i = 0; i < xor_len; i++)
		xor_sum ^= buf[i];

	return xor_sum;
}

int system_power_off(void)
{
	static console_t console;

	/* WT61P803 MCU system_off_now command */
	unsigned char system_off_now[4] = { '@', 'C', '0' };
	int i, len;

	len = sizeof(system_off_now);
	system_off_now[len - 1] = add_xor_checksum(system_off_now, len);

	console_16550_register(PLAT_MARVELL_UART_BASE + 0x100,
		PLAT_MARVELL_UART_CLK_IN_HZ, 115200, &console);

	/* Send system_off_now to console */
	for (i = 0; i < len; i++) {
		console.putc(system_off_now[i],	&console);
		udelay(1000);
	}

	console.flush(&console);
	(void)console_unregister(&console);

	mdelay(100);

	return 0;
}
