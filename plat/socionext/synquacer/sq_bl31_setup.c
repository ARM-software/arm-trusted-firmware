/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <platform_def.h>
#include <assert.h>
#include <bl_common.h>
#include <pl011.h>
#include <debug.h>

static console_pl011_t console;

void bl31_early_platform_setup(bl31_params_t *from_bl2,
				void *plat_params_from_bl2)
{
	/* Initialize the console to provide early debug support */
	(void)console_pl011_register(PLAT_SQ_BOOT_UART_BASE,
			       PLAT_SQ_BOOT_UART_CLK_IN_HZ,
			       SQ_CONSOLE_BAUDRATE, &console);

	console_set_scope(&console.console, CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME);

	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);
}

void bl31_platform_setup(void)
{
}

void bl31_plat_runtime_setup(void)
{
}

void bl31_plat_arch_setup(void)
{
}
