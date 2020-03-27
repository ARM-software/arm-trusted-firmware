/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/console.h>
#include <drivers/arm/pl011.h>

#include <platform_def.h>

static console_t console;

void fpga_console_init(void)
{
	(void)console_pl011_register(PLAT_FPGA_BOOT_UART_BASE,
		PLAT_FPGA_BOOT_UART_CLK_IN_HZ,
		PLAT_FPGA_CONSOLE_BAUDRATE,
		&console);

	console_set_scope(&console, CONSOLE_FLAG_BOOT |
		CONSOLE_FLAG_RUNTIME);
}
