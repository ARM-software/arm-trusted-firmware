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

int s32cc_init_early_clks(void)
{
	int ret;

	s32cc_clk_register_drv();

	ret = clk_set_rate(S32CC_CLK_FXOSC, S32CC_FXOSC_FREQ, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = clk_enable(S32CC_CLK_FXOSC);
	if (ret != 0) {
		return ret;
	}

	return ret;
}
