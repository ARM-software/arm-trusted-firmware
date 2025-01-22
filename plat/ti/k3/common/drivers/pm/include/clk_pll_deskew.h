/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLK_PLL_DESKEW_H
#define CLK_PLL_DESKEW_H

#include <clk_pll.h>

struct clk_data_pll_deskew {
	struct clk_data_pll	data_pll;
	uint32_t			base;
	uint8_t			idx;
};

extern const struct clk_drv clk_drv_pll_deskew;


#endif
