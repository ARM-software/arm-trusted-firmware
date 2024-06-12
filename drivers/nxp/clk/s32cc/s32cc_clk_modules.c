/*
 * Copyright 2020-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <s32cc-clk-ids.h>
#include <s32cc-clk-modules.h>
#include <s32cc-clk-utils.h>

#define S32CC_A53_MIN_FREQ	(48UL * MHZ)
#define S32CC_A53_MAX_FREQ	(1000UL * MHZ)

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

/* ARM PLL */
static struct s32cc_clkmux arm_pll_mux =
	S32CC_CLKMUX_INIT(S32CC_ARM_PLL, 0, 2,
			  S32CC_CLK_FIRC,
			  S32CC_CLK_FXOSC, 0, 0, 0);
static struct s32cc_clk arm_pll_mux_clk =
	S32CC_MODULE_CLK(arm_pll_mux);
static struct s32cc_pll armpll =
	S32CC_PLL_INIT(arm_pll_mux_clk, S32CC_ARM_PLL, 2);
static struct s32cc_clk arm_pll_vco_clk =
	S32CC_FREQ_MODULE_CLK(armpll, 1400 * MHZ, 2000 * MHZ);

static struct s32cc_pll_out_div arm_pll_phi0_div =
	S32CC_PLL_OUT_DIV_INIT(armpll, 0);
static struct s32cc_clk arm_pll_phi0_clk =
	S32CC_FREQ_MODULE_CLK(arm_pll_phi0_div, 0, GHZ);

/* MC_CGM1 */
static struct s32cc_clkmux cgm1_mux0 =
	S32CC_SHARED_CLKMUX_INIT(S32CC_CGM1, 0, 3,
				 S32CC_CLK_FIRC,
				 S32CC_CLK_ARM_PLL_PHI0,
				 S32CC_CLK_ARM_PLL_DFS2, 0, 0);
static struct s32cc_clk cgm1_mux0_clk = S32CC_MODULE_CLK(cgm1_mux0);

/* A53_CORE */
static struct s32cc_clk a53_core_clk =
	S32CC_FREQ_MODULE_CLK(cgm1_mux0_clk, S32CC_A53_MIN_FREQ,
			      S32CC_A53_MAX_FREQ);
/* A53_CORE_DIV2 */
static struct s32cc_fixed_div a53_core_div2 =
		S32CC_FIXED_DIV_INIT(cgm1_mux0_clk, 2);
static struct s32cc_clk a53_core_div2_clk =
	S32CC_FREQ_MODULE_CLK(a53_core_div2, S32CC_A53_MIN_FREQ / 2,
			      S32CC_A53_MAX_FREQ / 2);
/* A53_CORE_DIV10 */
static struct s32cc_fixed_div a53_core_div10 =
	S32CC_FIXED_DIV_INIT(cgm1_mux0_clk, 10);
static struct s32cc_clk a53_core_div10_clk =
	S32CC_FREQ_MODULE_CLK(a53_core_div10, S32CC_A53_MIN_FREQ / 10,
			      S32CC_A53_MAX_FREQ / 10);

static struct s32cc_clk *s32cc_hw_clk_list[5] = {
	/* Oscillators */
	[S32CC_CLK_ID(S32CC_CLK_FIRC)] = &firc_clk,
	[S32CC_CLK_ID(S32CC_CLK_SIRC)] = &sirc_clk,
	[S32CC_CLK_ID(S32CC_CLK_FXOSC)] = &fxosc_clk,
	/* ARM PLL */
	[S32CC_CLK_ID(S32CC_CLK_ARM_PLL_PHI0)] = &arm_pll_phi0_clk,
};

static struct s32cc_clk_array s32cc_hw_clocks = {
	.type_mask = S32CC_CLK_TYPE(S32CC_CLK_FIRC),
	.clks = &s32cc_hw_clk_list[0],
	.n_clks = ARRAY_SIZE(s32cc_hw_clk_list),
};

static struct s32cc_clk *s32cc_arch_clk_list[6] = {
	/* ARM PLL */
	[S32CC_CLK_ID(S32CC_CLK_ARM_PLL_MUX)] = &arm_pll_mux_clk,
	[S32CC_CLK_ID(S32CC_CLK_ARM_PLL_VCO)] = &arm_pll_vco_clk,
	/* MC_CGM1 */
	[S32CC_CLK_ID(S32CC_CLK_MC_CGM1_MUX0)] = &cgm1_mux0_clk,
	/* A53 */
	[S32CC_CLK_ID(S32CC_CLK_A53_CORE)] = &a53_core_clk,
	[S32CC_CLK_ID(S32CC_CLK_A53_CORE_DIV2)] = &a53_core_div2_clk,
	[S32CC_CLK_ID(S32CC_CLK_A53_CORE_DIV10)] = &a53_core_div10_clk,
};

static struct s32cc_clk_array s32cc_arch_clocks = {
	.type_mask = S32CC_CLK_TYPE(S32CC_CLK_ARM_PLL_MUX),
	.clks = &s32cc_arch_clk_list[0],
	.n_clks = ARRAY_SIZE(s32cc_arch_clk_list),
};

struct s32cc_clk *s32cc_get_arch_clk(unsigned long id)
{
	static const struct s32cc_clk_array *clk_table[2] = {
		&s32cc_hw_clocks,
		&s32cc_arch_clocks,
	};

	return s32cc_get_clk_from_table(clk_table, ARRAY_SIZE(clk_table), id);
}
