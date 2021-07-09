/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
static console_t arm_trp_runtime_console;

void arm_trp_early_platform_setup(void)
{
	/*
	 * Initialize a different console than already in use to display
	 * messages from trp
	 */
	int rc = console_pl011_register(PLAT_ARM_TRP_UART_BASE,
					PLAT_ARM_TRP_UART_CLK_IN_HZ,
					ARM_CONSOLE_BAUDRATE,
					&arm_trp_runtime_console);
	if (rc == 0) {
		panic();
	}

	console_set_scope(&arm_trp_runtime_console,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);
}

void trp_early_platform_setup(void)
{
	arm_trp_early_platform_setup();
}
