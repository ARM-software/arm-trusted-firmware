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

/* Partitions */
static struct s32cc_part part0 = S32CC_PART(0);

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

/* ARM DFS */
static struct s32cc_dfs armdfs =
	S32CC_DFS_INIT(armpll, S32CC_ARM_DFS);
static struct s32cc_dfs_div arm_dfs1_div =
	S32CC_DFS_DIV_INIT(armdfs, 0);
static struct s32cc_clk arm_dfs1_clk =
	S32CC_FREQ_MODULE_CLK(arm_dfs1_div, 0, 800 * MHZ);

/* MC_CGM0 */
static struct s32cc_clkmux cgm0_mux0 =
	S32CC_SHARED_CLKMUX_INIT(S32CC_CGM0, 0, 2,
				 S32CC_CLK_FIRC,
				 S32CC_CLK_ARM_PLL_DFS1, 0, 0, 0);
static struct s32cc_clk cgm0_mux0_clk = S32CC_MODULE_CLK(cgm0_mux0);

static struct s32cc_clkmux cgm0_mux8 =
	S32CC_SHARED_CLKMUX_INIT(S32CC_CGM0, 8, 3,
				 S32CC_CLK_FIRC,
				 S32CC_CLK_PERIPH_PLL_PHI3,
				 S32CC_CLK_FXOSC, 0, 0);
static struct s32cc_clk cgm0_mux8_clk = S32CC_MODULE_CLK(cgm0_mux8);

/* XBAR */
static struct s32cc_clk xbar_2x_clk =
	S32CC_CHILD_CLK(cgm0_mux0_clk, 48 * MHZ, 800 * MHZ);
static struct s32cc_fixed_div xbar_div2 =
	S32CC_FIXED_DIV_INIT(cgm0_mux0_clk, 2);
static struct s32cc_clk xbar_clk =
	S32CC_FREQ_MODULE_CLK(xbar_div2, 24 * MHZ, 400 * MHZ);
static struct s32cc_fixed_div xbar_div4 =
	S32CC_FIXED_DIV_INIT(cgm0_mux0_clk, 4);
static struct s32cc_clk xbar_div2_clk =
	S32CC_FREQ_MODULE_CLK(xbar_div4, 12 * MHZ, 200 * MHZ);
static struct s32cc_fixed_div xbar_div6 =
	S32CC_FIXED_DIV_INIT(cgm0_mux0_clk, 6);
static struct s32cc_clk xbar_div3_clk =
	S32CC_FREQ_MODULE_CLK(xbar_div6, 8 * MHZ, 133333333);
static struct s32cc_fixed_div xbar_div8 =
	S32CC_FIXED_DIV_INIT(cgm0_mux0_clk, 8);
static struct s32cc_clk xbar_div4_clk =
	S32CC_FREQ_MODULE_CLK(xbar_div8, 6 * MHZ, 100 * MHZ);
static struct s32cc_fixed_div xbar_div12 =
	S32CC_FIXED_DIV_INIT(cgm0_mux0_clk, 12);
static struct s32cc_clk xbar_div6_clk =
	S32CC_FREQ_MODULE_CLK(xbar_div12, 4 * MHZ, 66666666);

/* Linflex */
static struct s32cc_clk linflex_baud_clk =
	S32CC_CHILD_CLK(cgm0_mux8_clk, 19200, 133333333);
static struct s32cc_fixed_div linflex_div =
	S32CC_FIXED_DIV_INIT(linflex_baud_clk, 2);
static struct s32cc_clk linflex_clk =
	S32CC_FREQ_MODULE_CLK(linflex_div, 9600, 66666666);

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

/* PERIPH PLL */
static struct s32cc_clkmux periph_pll_mux =
	S32CC_CLKMUX_INIT(S32CC_PERIPH_PLL, 0, 2,
			  S32CC_CLK_FIRC,
			  S32CC_CLK_FXOSC, 0, 0, 0);
static struct s32cc_clk periph_pll_mux_clk =
	S32CC_MODULE_CLK(periph_pll_mux);
static struct s32cc_pll periphpll =
	S32CC_PLL_INIT(periph_pll_mux_clk, S32CC_PERIPH_PLL, 2);
static struct s32cc_clk periph_pll_vco_clk =
	S32CC_FREQ_MODULE_CLK(periphpll, 1300 * MHZ, 2 * GHZ);

static struct s32cc_pll_out_div periph_pll_phi3_div =
	S32CC_PLL_OUT_DIV_INIT(periphpll, 3);
static struct s32cc_clk periph_pll_phi3_clk =
	S32CC_FREQ_MODULE_CLK(periph_pll_phi3_div, 0, 133333333);

/* DDR PLL */
static struct s32cc_clkmux ddr_pll_mux =
	S32CC_CLKMUX_INIT(S32CC_DDR_PLL, 0, 2,
			  S32CC_CLK_FIRC,
			  S32CC_CLK_FXOSC, 0, 0, 0);
static struct s32cc_clk ddr_pll_mux_clk =
	S32CC_MODULE_CLK(ddr_pll_mux);
static struct s32cc_pll ddrpll =
	S32CC_PLL_INIT(ddr_pll_mux_clk, S32CC_DDR_PLL, 1);
static struct s32cc_clk ddr_pll_vco_clk =
	S32CC_FREQ_MODULE_CLK(ddrpll, 1300 * MHZ, 1600 * MHZ);

static struct s32cc_pll_out_div ddr_pll_phi0_div =
	S32CC_PLL_OUT_DIV_INIT(ddrpll, 0);
static struct s32cc_clk ddr_pll_phi0_clk =
	S32CC_FREQ_MODULE_CLK(ddr_pll_phi0_div, 0, 800 * MHZ);

/* MC_CGM5 */
static struct s32cc_clkmux cgm5_mux0 =
	S32CC_SHARED_CLKMUX_INIT(S32CC_CGM5, 0, 2,
				 S32CC_CLK_FIRC,
				 S32CC_CLK_DDR_PLL_PHI0,
				 0, 0, 0);
static struct s32cc_clk cgm5_mux0_clk = S32CC_MODULE_CLK(cgm5_mux0);

/* DDR clock */
static struct s32cc_part_block part0_block1 =
	S32CC_PART_BLOCK(&part0, s32cc_part_block1);
static struct s32cc_part_block_link ddr_block_link =
	S32CC_PART_BLOCK_LINK(cgm5_mux0_clk, &part0_block1);
static struct s32cc_clk ddr_clk =
	S32CC_FREQ_MODULE_CLK(ddr_block_link, 0, 800 * MHZ);

static struct s32cc_clk *s32cc_hw_clk_list[37] = {
	/* Oscillators */
	[S32CC_CLK_ID(S32CC_CLK_FIRC)] = &firc_clk,
	[S32CC_CLK_ID(S32CC_CLK_SIRC)] = &sirc_clk,
	[S32CC_CLK_ID(S32CC_CLK_FXOSC)] = &fxosc_clk,
	/* ARM PLL */
	[S32CC_CLK_ID(S32CC_CLK_ARM_PLL_PHI0)] = &arm_pll_phi0_clk,
	/* ARM DFS */
	[S32CC_CLK_ID(S32CC_CLK_ARM_PLL_DFS1)] = &arm_dfs1_clk,
	/* PERIPH PLL */
	[S32CC_CLK_ID(S32CC_CLK_PERIPH_PLL_PHI3)] = &periph_pll_phi3_clk,
	/* DDR PLL */
	[S32CC_CLK_ID(S32CC_CLK_DDR_PLL_PHI0)] = &ddr_pll_phi0_clk,
};

static struct s32cc_clk_array s32cc_hw_clocks = {
	.type_mask = S32CC_CLK_TYPE(S32CC_CLK_FIRC),
	.clks = &s32cc_hw_clk_list[0],
	.n_clks = ARRAY_SIZE(s32cc_hw_clk_list),
};

static struct s32cc_clk *s32cc_arch_clk_list[22] = {
	/* ARM PLL */
	[S32CC_CLK_ID(S32CC_CLK_ARM_PLL_MUX)] = &arm_pll_mux_clk,
	[S32CC_CLK_ID(S32CC_CLK_ARM_PLL_VCO)] = &arm_pll_vco_clk,
	/* PERIPH PLL */
	[S32CC_CLK_ID(S32CC_CLK_PERIPH_PLL_MUX)] = &periph_pll_mux_clk,
	[S32CC_CLK_ID(S32CC_CLK_PERIPH_PLL_VCO)] = &periph_pll_vco_clk,
	/* MC_CGM0 */
	[S32CC_CLK_ID(S32CC_CLK_MC_CGM0_MUX0)] = &cgm0_mux0_clk,
	[S32CC_CLK_ID(S32CC_CLK_MC_CGM0_MUX8)] = &cgm0_mux8_clk,
	/* XBAR */
	[S32CC_CLK_ID(S32CC_CLK_XBAR_2X)] = &xbar_2x_clk,
	[S32CC_CLK_ID(S32CC_CLK_XBAR)] = &xbar_clk,
	[S32CC_CLK_ID(S32CC_CLK_XBAR_DIV2)] = &xbar_div2_clk,
	[S32CC_CLK_ID(S32CC_CLK_XBAR_DIV3)] = &xbar_div3_clk,
	[S32CC_CLK_ID(S32CC_CLK_XBAR_DIV4)] = &xbar_div4_clk,
	[S32CC_CLK_ID(S32CC_CLK_XBAR_DIV6)] = &xbar_div6_clk,
	/* MC_CGM1 */
	[S32CC_CLK_ID(S32CC_CLK_MC_CGM1_MUX0)] = &cgm1_mux0_clk,
	/* A53 */
	[S32CC_CLK_ID(S32CC_CLK_A53_CORE)] = &a53_core_clk,
	[S32CC_CLK_ID(S32CC_CLK_A53_CORE_DIV2)] = &a53_core_div2_clk,
	[S32CC_CLK_ID(S32CC_CLK_A53_CORE_DIV10)] = &a53_core_div10_clk,
	/* Linflex */
	[S32CC_CLK_ID(S32CC_CLK_LINFLEX)] = &linflex_clk,
	[S32CC_CLK_ID(S32CC_CLK_LINFLEX_BAUD)] = &linflex_baud_clk,
	/* DDR PLL */
	[S32CC_CLK_ID(S32CC_CLK_DDR_PLL_MUX)] = &ddr_pll_mux_clk,
	[S32CC_CLK_ID(S32CC_CLK_DDR_PLL_VCO)] = &ddr_pll_vco_clk,
	/* MC_CGM5 */
	[S32CC_CLK_ID(S32CC_CLK_MC_CGM5_MUX0)] = &cgm5_mux0_clk,
	/* DDR */
	[S32CC_CLK_ID(S32CC_CLK_DDR)] = &ddr_clk,
};

static struct s32cc_clk_array s32cc_arch_clocks = {
	.type_mask = S32CC_CLK_TYPE(S32CC_CLK_ARM_PLL_MUX),
	.clks = &s32cc_arch_clk_list[0],
	.n_clks = ARRAY_SIZE(s32cc_arch_clk_list),
};

static const struct s32cc_clk_array *s32cc_clk_table[2] = {
	&s32cc_hw_clocks,
	&s32cc_arch_clocks,
};

struct s32cc_clk *s32cc_get_arch_clk(unsigned long id)
{
	return s32cc_get_clk_from_table(s32cc_clk_table,
					ARRAY_SIZE(s32cc_clk_table),
					id);
}

int s32cc_get_clk_id(const struct s32cc_clk *clk, unsigned long *id)
{
	return s32cc_get_id_from_table(s32cc_clk_table,
				       ARRAY_SIZE(s32cc_clk_table),
				       clk, id);
}
