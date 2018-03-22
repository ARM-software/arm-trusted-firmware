/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <console.h>
#include <pl011.h>
#include <platform_def.h>

static console_pl011_t console;
static console_pl011_t crash_console;

void qemu_console_init(void)
{
#if MULTI_CONSOLE_API
	(void)console_pl011_register(PLAT_QEMU_BOOT_UART_BASE,
			       PLAT_QEMU_BOOT_UART_CLK_IN_HZ,
			       PLAT_QEMU_CONSOLE_BAUDRATE, &console);
#else
	console_init(PLAT_QEMU_BOOT_UART_BASE,
		     PLAT_QEMU_BOOT_UART_CLK_IN_HZ,
		     PLAT_QEMU_CONSOLE_BAUDRATE);
#endif /* MULTI_CONSOLE_API */
}

void qemu_crash_console_init(void)
{
#if MULTI_CONSOLE_API
	(void)console_pl011_register(PLAT_QEMU_CRASH_UART_BASE,
			       PLAT_QEMU_CRASH_UART_CLK_IN_HZ,
			       PLAT_QEMU_CONSOLE_BAUDRATE, &crash_console);
#else
	console_core_init(PLAT_QEMU_CRASH_UART_BASE,
			  PLAT_QEMU_CRASH_UART_CLK_IN_HZ,
			  PLAT_QEMU_CONSOLE_BAUDRATE);
#endif /* MULTI_CONSOLE_API */
}
