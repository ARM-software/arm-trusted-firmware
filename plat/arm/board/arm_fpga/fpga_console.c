/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <common/fdt_wrappers.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>

#include <platform_def.h>

static console_t console;

void fpga_console_init(void)
{
	const void *fdt = (void *)(uintptr_t)FPGA_PRELOADED_DTB_BASE;
	uintptr_t base_addr = PLAT_FPGA_CRASH_UART_BASE;
	int node;

	/*
	 * Try to read the UART base address from the DT, by chasing the
	 * stdout-path property of the chosen node.
	 * If this does not work, use the crash console address as a fallback.
	 */
	node = fdt_get_stdout_node_offset(fdt);
	if (node >= 0) {
		fdt_get_reg_props_by_index(fdt, node, 0, &base_addr, NULL);
	}

	(void)console_pl011_register(base_addr, 0, 0, &console);

	console_set_scope(&console, CONSOLE_FLAG_BOOT |
		CONSOLE_FLAG_RUNTIME);
}
