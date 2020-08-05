/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/ti/uart/uart_16550.h>

#include <platform_def.h>

/*******************************************************************************
 * Functions that set up the console
 ******************************************************************************/
static console_t bcm_boot_console;
static console_t bcm_runtime_console;

/* Initialize the console to provide early debug support */
void bcm_console_boot_init(void)
{
	int rc = console_16550_register(PLAT_BRCM_BOOT_UART_BASE,
					PLAT_BRCM_BOOT_UART_CLK_IN_HZ,
					BRCM_CONSOLE_BAUDRATE,
					&bcm_boot_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}

	console_set_scope(&bcm_boot_console, CONSOLE_FLAG_BOOT);
}

void bcm_console_boot_end(void)
{
	console_flush();

	(void)console_unregister(&bcm_boot_console);
}

/* Initialize the runtime console */
void bcm_console_runtime_init(void)
{
	int rc = console_16550_register(PLAT_BRCM_BL31_RUN_UART_BASE,
					PLAT_BRCM_BL31_RUN_UART_CLK_IN_HZ,
					BRCM_CONSOLE_BAUDRATE,
					&bcm_runtime_console);
	if (rc == 0)
		panic();

	console_set_scope(&bcm_runtime_console, CONSOLE_FLAG_RUNTIME);
}

void bcm_console_runtime_end(void)
{
	console_flush();

	(void)console_unregister(&bcm_runtime_console);
}
