/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLK_ADPLLM_H
#define CLK_ADPLLM_H

#include <clk_pll.h>
#include <clk_div.h>

struct clk_data_pll_adpllm {
	struct clk_data_pll	data_pll;
	uint32_t			base;
	bool			hsdiv;
	bool			ljm;
	uint32_t			idx;
};

extern const struct clk_drv clk_drv_adpllm;
extern const struct clk_drv_mux clk_drv_adpllm_bypass_mux;

struct clk_data_hsdiv {
	struct clk_data_div_reg data_div_reg;
	uint8_t			go_bit : 5;
	uint8_t			idx : 3;
	uint32_t			go_reg;
};

extern const struct clk_drv_div clk_drv_div_hsdiv;
extern const struct clk_drv_div clk_drv_div_hsdiv4;

#endif
