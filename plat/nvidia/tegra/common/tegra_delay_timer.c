/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <delay_timer.h>
#include <mmio.h>
#include <tegra_def.h>

static uint32_t tegra_timerus_get_value(void)
{
	return mmio_read_32(TEGRA_TMRUS_BASE);
}

static const timer_ops_t tegra_timer_ops = {
	.get_timer_value	= tegra_timerus_get_value,
	.clk_mult		= 1,
	.clk_div		= 1,
};

/*
 * Initialise the on-chip free rolling us counter as the delay
 * timer.
 */
void tegra_delay_timer_init(void)
{
	timer_init(&tegra_timer_ops);
}
