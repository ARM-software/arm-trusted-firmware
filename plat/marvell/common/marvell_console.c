/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/console.h>

#include <plat_marvell.h>

#ifdef PLAT_a3700
#include <drivers/marvell/uart/a3700_console.h>

static console_a3700_t marvell_boot_console;
static console_a3700_t marvell_runtime_console;
#else
#include <drivers/ti/uart/uart_16550.h>

static console_16550_t marvell_boot_console;
static console_16550_t marvell_runtime_console;
#endif

/*******************************************************************************
 * Functions that set up the console
 ******************************************************************************/

/* Initialize the console to provide early debug support */
void marvell_console_boot_init(void)
{
	int rc =
#ifdef PLAT_a3700
	console_a3700_register(
#else
	console_16550_register(
#endif
				PLAT_MARVELL_BOOT_UART_BASE,
				PLAT_MARVELL_BOOT_UART_CLK_IN_HZ,
				MARVELL_CONSOLE_BAUDRATE,
				&marvell_boot_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}

	console_set_scope(&marvell_boot_console.console,
			  CONSOLE_FLAG_BOOT);
}

void marvell_console_boot_end(void)
{
	(void)console_flush();

	(void)console_unregister(&marvell_boot_console.console);
}

/* Initialize the runtime console */
void marvell_console_runtime_init(void)
{
	int rc =
#ifdef PLAT_a3700
	console_a3700_register(
#else
	console_16550_register(
#endif
				PLAT_MARVELL_BOOT_UART_BASE,
				PLAT_MARVELL_BOOT_UART_CLK_IN_HZ,
				MARVELL_CONSOLE_BAUDRATE,
				&marvell_runtime_console);
	if (rc == 0)
		panic();

	console_set_scope(&marvell_runtime_console.console,
			  CONSOLE_FLAG_RUNTIME);
}

void marvell_console_runtime_end(void)
{
	(void)console_flush();

	(void)console_unregister(&marvell_runtime_console.console);
}
