/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <common/debug.h>
#include <dcfg.h>
#include <lib/utils.h>
#include <plat_console.h>

/*
 * Perform Arm specific early platform setup. At this moment we only initialize
 * the console and the memory layout.
 */
void plat_console_init(uintptr_t nxp_console_addr, uint32_t uart_clk_div,
			uint32_t baud)
{
	struct sysinfo sys;
	static console_t nxp_console;

	zeromem(&sys, sizeof(sys));
	if (get_clocks(&sys)) {
		ERROR("System clocks are not set\n");
		panic();
	}
	nxp_console_16550_register(nxp_console_addr,
			      (sys.freq_platform/uart_clk_div),
			       baud, &nxp_console);
}
