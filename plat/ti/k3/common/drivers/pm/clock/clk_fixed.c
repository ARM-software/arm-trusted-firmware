/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_fixed.h>
#include <lib/container_of.h>
#include <lib/io.h>
#include <compiler.h>

static uint32_t clk_fixed_get_freq(struct clk *clkp)
{
	const struct clk_data *clk_datap;
	const struct clk_range *range;
	uint32_t ret = 0;


	clk_datap = clk_get_data(clkp);
	range = clk_get_range(clk_datap->range_idx);
	if (range != NULL) {
		ret = range->min_hz;
	}

	return ret;
}

static uint32_t clk_fixed_get_state(struct clk *clkp UNUSED)
{
	return CLK_HW_STATE_ENABLED;
}

const struct clk_drv clk_drv_fixed = {
	.get_freq	= clk_fixed_get_freq,
	.get_state	= clk_fixed_get_state,
};
