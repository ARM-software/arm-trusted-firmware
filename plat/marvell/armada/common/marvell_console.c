/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
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
#define PLAT_MARVELL_UART_CLK_IN_HZ (get_ref_clk() * 1000000)
#define console_marvell_register console_a3700_register
#else
#include <drivers/ti/uart/uart_16550.h>
#define console_marvell_register console_16550_register
#endif

static console_t marvell_boot_console;
static console_t marvell_runtime_console;

/*******************************************************************************
 * Functions that set up the console
 ******************************************************************************/

/* Initialize the console to provide early debug support */
void marvell_console_boot_init(void)
{
	int rc =
	console_marvell_register(PLAT_MARVELL_UART_BASE,
				 PLAT_MARVELL_UART_CLK_IN_HZ,
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

	console_set_scope(&marvell_boot_console, CONSOLE_FLAG_BOOT);
}

void marvell_console_boot_end(void)
{
	console_flush();

	(void)console_unregister(&marvell_boot_console);
}

/* Initialize the runtime console */
void marvell_console_runtime_init(void)
{
	int rc =
	console_marvell_register(PLAT_MARVELL_UART_BASE,
				 PLAT_MARVELL_UART_CLK_IN_HZ,
				 MARVELL_CONSOLE_BAUDRATE,
				 &marvell_runtime_console);
	if (rc == 0)
		panic();

	console_set_scope(&marvell_runtime_console, CONSOLE_FLAG_RUNTIME);
}

void marvell_console_runtime_end(void)
{
	console_flush();

	(void)console_unregister(&marvell_runtime_console);
}
