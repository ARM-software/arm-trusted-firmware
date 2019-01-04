/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <imx_regs.h>
#include <imx_wdog.h>

static void imx_wdog_power_down(unsigned long base)
{
	struct wdog_regs *wdog = (struct wdog_regs *)base;

	mmio_write_16((uintptr_t)&wdog->wmcr, 0);
}

void imx_wdog_init(void)
{
	imx_wdog_power_down(WDOG1_BASE);
	imx_wdog_power_down(WDOG2_BASE);
	imx_wdog_power_down(WDOG3_BASE);
	imx_wdog_power_down(WDOG4_BASE);
}
