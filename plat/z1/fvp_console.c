/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <fconf_hw_config_getter.h>
#include <plat/arm/common/plat_arm.h>

static console_t fvp_runtime_console;

/* Initialize the runtime console */
void arm_console_runtime_init(void)
{
	uintptr_t uart_base;
	uint32_t uart_clk;

	/*
	 * fconf APIs are not supported for RESET_TO_SP_MIN, RESET_TO_BL31 and
	 * BL2_AT_EL3 systems.
	 */
#if RESET_TO_SP_MIN || RESET_TO_BL31 || BL2_AT_EL3
	uart_base = PLAT_ARM_RUN_UART_BASE;
	uart_clk = PLAT_ARM_RUN_UART_CLK_IN_HZ;
#else
	uart_base = FCONF_GET_PROPERTY(hw_config, uart_serial_config,
					uart_base);
	uart_clk = FCONF_GET_PROPERTY(hw_config, uart_serial_config,
					uart_clk);
#endif

	int rc = console_pl011_register(uart_base, uart_clk,
					ARM_CONSOLE_BAUDRATE,
					&fvp_runtime_console);

	if (rc == 0) {
		panic();
	}

	console_set_scope(&fvp_runtime_console, CONSOLE_FLAG_RUNTIME);
}

void arm_console_runtime_end(void)
{
	console_flush();
	(void)console_unregister(&fvp_runtime_console);
}
