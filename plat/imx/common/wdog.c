/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <imx-regs.h>
#include <wdog.h>
#include <mmio.h>

#include <debug.h>

static void wdog_power_down(unsigned long base)
{
	struct wdog_regs *wdog = (struct wdog_regs *)base;

	mmio_write_16((uintptr_t)&wdog->wmcr, 0);
}

void wdog_init(void)
{
	wdog_power_down(WDOG1_BASE);
	wdog_power_down(WDOG2_BASE);
	wdog_power_down(WDOG3_BASE);
	wdog_power_down(WDOG4_BASE);
}
