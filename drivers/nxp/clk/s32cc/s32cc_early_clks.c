/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <drivers/clk.h>
#include <s32cc-clk-drv.h>
#include <s32cc-clk-ids.h>
#include <s32cc-clk-utils.h>

#define S32CC_FXOSC_FREQ	(40U * MHZ)
#define S32CC_ARM_PLL_VCO_FREQ	(2U * GHZ)
#define S32CC_ARM_PLL_PHI0_FREQ	(1U * GHZ)

int s32cc_init_early_clks(void)
{
	int ret;

	s32cc_clk_register_drv();

	ret = clk_set_parent(S32CC_CLK_ARM_PLL_MUX, S32CC_CLK_FXOSC);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_parent(S32CC_CLK_MC_CGM1_MUX0, S32CC_CLK_ARM_PLL_PHI0);
	if (ret != 0) {
		return ret;
	}

	ret = clk_set_rate(S32CC_CLK_FXOSC, S32CC_FXOSC_FREQ, NULL);
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

	ret = clk_enable(S32CC_CLK_FXOSC);
	if (ret != 0) {
		return ret;
	}

	return ret;
}
