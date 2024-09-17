/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <drivers/clk.h>
#include <platform_def.h>
#include <s32cc-clk-drv.h>
#include <s32cc-clk-ids.h>
#include <s32cc-clk-utils.h>

#define S32CC_FXOSC_FREQ		(40U * MHZ)
#define S32CC_ARM_PLL_VCO_FREQ		(2U * GHZ)
#define S32CC_ARM_PLL_PHI0_FREQ		(1U * GHZ)
#define S32CC_A53_FREQ			(1U * GHZ)
#define S32CC_XBAR_2X_FREQ		(800U * MHZ)
#define S32CC_PERIPH_PLL_VCO_FREQ	(2U * GHZ)
#define S32CC_PERIPH_PLL_PHI3_FREQ	UART_CLOCK_HZ
#define S32CC_DDR_PLL_VCO_FREQ		(1600U * MHZ)
#define S32CC_DDR_PLL_PHI0_FREQ		(800U * MHZ)

static int setup_fxosc(void)
{
	int ret;

	ret = clk_set_rate(S32CC_CLK_FXOSC, S32CC_FXOSC_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int setup_arm_pll(void)
{
	int ret;

	ret = clk_set_parent(S32CC_CLK_ARM_PLL_MUX, S32CC_CLK_FXOSC);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_ARM_PLL_VCO, S32CC_ARM_PLL_VCO_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_ARM_PLL_PHI0, S32CC_ARM_PLL_PHI0_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int setup_periph_pll(void)
{
	int ret;

	ret = clk_set_parent(S32CC_CLK_PERIPH_PLL_MUX, S32CC_CLK_FXOSC);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_PERIPH_PLL_VCO, S32CC_PERIPH_PLL_VCO_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_PERIPH_PLL_PHI3, S32CC_PERIPH_PLL_PHI3_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int enable_a53_clk(void)
{
	int ret;

	ret = clk_set_parent(S32CC_CLK_MC_CGM1_MUX0, S32CC_CLK_ARM_PLL_PHI0);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_A53_CORE, S32CC_A53_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = clk_enable(S32CC_CLK_A53_CORE);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int enable_xbar_clk(void)
{
	int ret;

	ret = clk_set_parent(S32CC_CLK_MC_CGM0_MUX0, S32CC_CLK_ARM_PLL_DFS1);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_XBAR_2X, S32CC_XBAR_2X_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = clk_enable(S32CC_CLK_ARM_PLL_DFS1);
	if (ret != 0) {
		return ret;
	}

	ret = clk_enable(S32CC_CLK_XBAR_2X);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int enable_uart_clk(void)
{
	int ret;

	ret = clk_set_parent(S32CC_CLK_MC_CGM0_MUX8, S32CC_CLK_PERIPH_PLL_PHI3);
	if (ret != 0) {
		return ret;
	}

	ret = clk_enable(S32CC_CLK_LINFLEX_BAUD);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int setup_ddr_pll(void)
{
	int ret;

	ret = clk_set_parent(S32CC_CLK_DDR_PLL_MUX, S32CC_CLK_FXOSC);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_DDR_PLL_VCO, S32CC_DDR_PLL_VCO_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_DDR_PLL_PHI0, S32CC_DDR_PLL_PHI0_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

static int enable_ddr_clk(void)
{
	int ret;

	ret = clk_set_parent(S32CC_CLK_MC_CGM5_MUX0, S32CC_CLK_DDR_PLL_PHI0);
	if (ret != 0) {
		return ret;
	}

	ret = clk_enable(S32CC_CLK_DDR);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

int s32cc_init_early_clks(void)
{
	int ret;

	s32cc_clk_register_drv();

	ret = setup_fxosc();
	if (ret != 0) {
		return ret;
	}

	ret = setup_arm_pll();
	if (ret != 0) {
		return ret;
	}

	ret = enable_a53_clk();
	if (ret != 0) {
		return ret;
	}

	ret = enable_xbar_clk();
	if (ret != 0) {
		return ret;
	}

	ret = setup_periph_pll();
	if (ret != 0) {
		return ret;
	}

	ret = enable_uart_clk();
	if (ret != 0) {
		return ret;
	}

	ret = setup_ddr_pll();
	if (ret != 0) {
		return ret;
	}

	ret = enable_ddr_clk();
	if (ret != 0) {
		return ret;
	}

	return ret;
}
