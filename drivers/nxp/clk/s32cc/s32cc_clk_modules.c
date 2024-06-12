/*
 * Copyright 2020-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <s32cc-clk-ids.h>
#include <s32cc-clk-modules.h>
#include <s32cc-clk-utils.h>

/* Oscillators */
static struct s32cc_osc fxosc =
	S32CC_OSC_INIT(S32CC_FXOSC);
static struct s32cc_clk fxosc_clk =
	S32CC_MODULE_CLK(fxosc);

static struct s32cc_osc firc =
	S32CC_OSC_INIT(S32CC_FIRC);
static struct s32cc_clk firc_clk =
	S32CC_MODULE_CLK(firc);

static struct s32cc_osc sirc =
	S32CC_OSC_INIT(S32CC_SIRC);
static struct s32cc_clk sirc_clk =
	S32CC_MODULE_CLK(sirc);

static struct s32cc_clk *s32cc_hw_clk_list[3] = {
	/* Oscillators */
	[S32CC_CLK_ID(S32CC_CLK_FIRC)] = &firc_clk,
	[S32CC_CLK_ID(S32CC_CLK_SIRC)] = &sirc_clk,
	[S32CC_CLK_ID(S32CC_CLK_FXOSC)] = &fxosc_clk,
};

static struct s32cc_clk_array s32cc_hw_clocks = {
	.type_mask = S32CC_CLK_TYPE(S32CC_CLK_FIRC),
	.clks = &s32cc_hw_clk_list[0],
	.n_clks = ARRAY_SIZE(s32cc_hw_clk_list),
};

struct s32cc_clk *s32cc_get_arch_clk(unsigned long id)
{
	static const struct s32cc_clk_array *clk_table[1] = {
		&s32cc_hw_clocks,
	};

	return s32cc_get_clk_from_table(clk_table, ARRAY_SIZE(clk_table), id);
}
