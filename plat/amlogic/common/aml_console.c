/*
 * Copyright (c) 2019, Carlo Caione <ccaione@baylibre.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <meson_console.h>
#include <platform_def.h>

/*******************************************************************************
 * Function that sets up the console
 ******************************************************************************/
static console_meson_t aml_console;

void aml_console_init(void)
{
	int rc = console_meson_register(AML_UART0_AO_BASE,
					AML_UART0_AO_CLK_IN_HZ,
					AML_UART_BAUDRATE,
					&aml_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}

	console_set_scope(&aml_console.console,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);
}
