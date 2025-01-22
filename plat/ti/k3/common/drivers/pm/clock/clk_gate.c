/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_gate.h>
#include <clk_mux.h>
#include <lib/container_of.h>
#include <lib/io.h>

static bool clk_gate_set_state(struct clk *clkp, bool enable)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_reg *reg;
	uint32_t v;
	bool ret = true;

	reg = container_of(clk_datap->data, const struct clk_data_reg, data);

	v = readl(reg->reg);
	if (enable) {
		v |= BIT(reg->bit);
	} else {
		v &= ~BIT(reg->bit);
	}
	ti_clk_writel(v, reg->reg);

	return ret;
}

static uint32_t clk_gate_get_state(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_parent *p;
	struct clk *clk_parentp = NULL;
	uint32_t ret = CLK_HW_STATE_ENABLED;

	p = clk_get_parent(clkp);
	if (p != NULL) {
		clk_parentp = clk_lookup((clk_idx_t) p->clk);
	}

	if (clk_parentp != NULL) {
		ret = clk_get_state(clk_parentp);
	} else {
		/* No parent...cannot function */
		ret = CLK_HW_STATE_DISABLED;
	}

	if (ret == CLK_HW_STATE_ENABLED) {
		const struct clk_data_reg *reg;

		/* Parent is enabled, are we gating it? */
		reg = container_of(clk_datap->data, const struct clk_data_reg,
				   data);
		if (0U == (readl(reg->reg) & BIT(reg->bit))) {
			ret = CLK_HW_STATE_DISABLED;
		}
	}

	return ret;
}

const struct clk_drv clk_gate_drv_reg = {
	.set_state	= clk_gate_set_state,
	.get_state	= clk_gate_get_state,
};
