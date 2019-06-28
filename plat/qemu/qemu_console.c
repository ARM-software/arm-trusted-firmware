/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <drivers/console.h>
#include <drivers/arm/pl011.h>

static console_pl011_t console;

void qemu_console_init(void)
{
	(void)console_pl011_register(PLAT_QEMU_BOOT_UART_BASE,
			       PLAT_QEMU_BOOT_UART_CLK_IN_HZ,
			       PLAT_QEMU_CONSOLE_BAUDRATE, &console);

	console_set_scope(&console.console, CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME);
}

