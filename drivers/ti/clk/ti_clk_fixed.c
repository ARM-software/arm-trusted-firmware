/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Fixed Clock Driver
 *
 * This driver implements fixed-frequency clocks that cannot be modified.
 * Fixed clocks are always enabled and return their configured frequency
 * from the clock range data. These are typically used for board-level
 * reference clocks and other unchangeable clock sources.
 */

#include <cdefs.h>
#include <ti_clk_fixed.h>

static uint32_t ti_clk_fixed_get_freq(struct ti_clk *clkp)
{
	const struct ti_clk_range *range;

	range = ti_clk_get_range(clkp->range_idx);
	if (range == NULL) {
		return 0U;
	}

	return range->min_hz;
}

static uint32_t ti_clk_fixed_get_state(struct ti_clk *clkp __unused)
{
	return TI_CLK_HW_STATE_ENABLED;
}

const struct ti_clk_drv ti_clk_drv_fixed = {
	.get_freq = ti_clk_fixed_get_freq,
	.get_state = ti_clk_fixed_get_state,
};
