/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include "watchdog.h"


/* Reset watchdog timer */
void watchdog_sw_rst(void)
{
	mmio_write_32(WDT_CRR, WDT_SW_RST);
}

/* Print component information */
void watchdog_info(void)
{
	INFO("Component Type    : %x\r\n", mmio_read_32(WDT_COMP_VERSION));
	INFO("Component Version : %x\r\n", mmio_read_32(WDT_COMP_TYPE));
}

/* Check watchdog current status */
void watchdog_status(void)
{
	if (mmio_read_32(WDT_CR) & 1) {
		INFO("Watchdog Timer in currently enabled\n");
		INFO("Current Counter : 0x%x\r\n", mmio_read_32(WDT_CCVR));
	} else {
		INFO("Watchdog Timer in currently disabled\n");
	}
}

/* Initialize & enable watchdog */
void watchdog_init(int watchdog_clk)
{
	uint8_t cycles_i = 0;
	uint32_t wdt_cycles = WDT_MIN_CYCLES;
	uint32_t top_init_cycles = WDT_PERIOD * watchdog_clk;

	while ((cycles_i < 15) && (wdt_cycles < top_init_cycles)) {
		wdt_cycles = (wdt_cycles << 1);
		cycles_i++;
	}

	mmio_write_32(WDT_TORR, (cycles_i << 4) | cycles_i);

	watchdog_enable();
}

void watchdog_enable(void)
{
	mmio_write_32(WDT_CR, WDT_CR_RMOD|WDT_CR_EN);
}
