/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <pl011.h>
#include <plat_arm.h>
#include <platform_def.h>

/*******************************************************************************
 * Functions that set up the console
 ******************************************************************************/
#if MULTI_CONSOLE_API
static console_pl011_t arm_boot_console;
static console_pl011_t arm_runtime_console;
#endif

/* Initialize the console to provide early debug support */
void arm_console_boot_init(void)
{
#if MULTI_CONSOLE_API
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

	console_set_scope(&arm_boot_console.console, CONSOLE_FLAG_BOOT);
#else
	(void)console_init(PLAT_ARM_BOOT_UART_BASE,
			   PLAT_ARM_BOOT_UART_CLK_IN_HZ,
			   ARM_CONSOLE_BAUDRATE);
#endif /* MULTI_CONSOLE_API */
}

void arm_console_boot_end(void)
{
	(void)console_flush();

#if MULTI_CONSOLE_API
	(void)console_unregister(&arm_boot_console.console);
#else
	console_uninit();
#endif /* MULTI_CONSOLE_API */
}

/* Initialize the runtime console */
void arm_console_runtime_init(void)
{
#if MULTI_CONSOLE_API
	int rc = console_pl011_register(PLAT_ARM_BL31_RUN_UART_BASE,
					PLAT_ARM_BL31_RUN_UART_CLK_IN_HZ,
					ARM_CONSOLE_BAUDRATE,
					&arm_runtime_console);
	if (rc == 0)
		panic();

	console_set_scope(&arm_runtime_console.console, CONSOLE_FLAG_RUNTIME);
#else
	(void)console_init(PLAT_ARM_BL31_RUN_UART_BASE,
			   PLAT_ARM_BL31_RUN_UART_CLK_IN_HZ,
			   ARM_CONSOLE_BAUDRATE);
#endif /* MULTI_CONSOLE_API */
}

void arm_console_runtime_end(void)
{
	(void)console_flush();

#if MULTI_CONSOLE_API
	(void)console_unregister(&arm_runtime_console.console);
#else
	console_uninit();
#endif /* MULTI_CONSOLE_API */
}
