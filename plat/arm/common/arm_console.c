/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <plat/arm/common/plat_arm.h>

#pragma weak arm_console_runtime_init

/*******************************************************************************
 * Functions that set up the console
 ******************************************************************************/
static console_t arm_boot_console;
static console_t arm_runtime_console;

/* Initialize the console to provide early debug support */
void __init arm_console_boot_init(void)
{
	/* If the console was initialized already, don't initialize again */
	if (arm_boot_console.base == PLAT_ARM_BOOT_UART_BASE) {
		return;
	}

	int rc = console_pl011_register(PLAT_ARM_BOOT_UART_BASE,
					PLAT_ARM_BOOT_UART_CLK_IN_HZ,
					ARM_CONSOLE_BAUDRATE,
					&arm_boot_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}

	console_set_scope(&arm_boot_console, CONSOLE_FLAG_BOOT);
}

void arm_console_boot_end(void)
{
	console_flush();
	(void)console_unregister(&arm_boot_console);
}

/* Initialize the runtime console */
void arm_console_runtime_init(void)
{
	int rc = console_pl011_register(PLAT_ARM_RUN_UART_BASE,
					PLAT_ARM_RUN_UART_CLK_IN_HZ,
					ARM_CONSOLE_BAUDRATE,
					&arm_runtime_console);
	if (rc == 0)
		panic();

	console_set_scope(&arm_runtime_console, CONSOLE_FLAG_RUNTIME);
}

void arm_console_runtime_end(void)
{
	console_flush();
}
