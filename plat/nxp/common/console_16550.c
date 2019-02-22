/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <plat_common.h>

#include <drivers/ti/uart/uart_16550.h>
#include <lib/utils.h>

/*
 * Perform arm specific early platform setup. At this moment we only initialize
 * the console and the memory layout.
 */
void plat_console_init(void)
{
	static console_16550_t console;
	struct sysinfo sys;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);

	console_16550_register(NXP_CONSOLE_ADDR,
			      (sys.freq_platform/NXP_UART_CLK_DIVIDER),
			       NXP_CONSOLE_BAUDRATE, &console);
}
