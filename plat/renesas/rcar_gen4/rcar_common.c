/*
 * Copyright (c) 2019-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/console.h>
#include "scif.h"

#include "rcar_private.h"

/* RAS functions common to AArch64 ARM platforms */
void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		     void *handle, uint64_t flags)
{
}

void rcar_console_boot_init(void)
{
	static console_t rcar_boot_console = { 0 };
	int ret;

	ret = console_rcar_register(0, 0, 0, &rcar_boot_console);
	if (ret == 0)
		panic();

	console_set_scope(&rcar_boot_console, CONSOLE_FLAG_BOOT);
}

void rcar_console_runtime_init(void)
{
	static console_t rcar_runtime_console = { 0 };
	int ret;

	ret = console_rcar_register(1, 0, 0, &rcar_runtime_console);
	if (ret == 0)
		panic();

	console_set_scope(&rcar_runtime_console,
			  CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME |
			  CONSOLE_FLAG_CRASH);
}
