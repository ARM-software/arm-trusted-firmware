/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLK_DIV_H
#define CLK_DIV_H

#include <clk.h>

struct clk_data_div {
	struct clk_drv_data	data;
	uint16_t			n;
	uint16_t			default_div;
};

struct clk_data_div_reg {
	uint32_t			reg;
	struct clk_data_div	data_div;
	uint8_t			bit : 7;
	uint8_t			start_at_1 : 1;
};

struct clk_data_div_reg_go {
	uint32_t			reg;
	struct clk_data_div	data_div;
	uint8_t			bit : 7;
	uint8_t			start_at_1 : 1;
	uint8_t			go;
};

struct clk_drv_div {
	struct clk_drv	drv;
	bool		(*set_div)(struct clk *clkp, uint32_t d);
	uint32_t		(*get_div)(struct clk *clkp);
	bool		(*valid_div)(struct clk *clkp, uint32_t d);
};

extern const struct clk_drv_div clk_drv_div_reg;
extern const struct clk_drv_div clk_drv_div_reg_ro;
extern const struct clk_drv_div clk_drv_div_reg_go;
extern const struct clk_drv_div clk_drv_div_fixed;

bool clk_div_notify_freq(struct clk *clkp, uint32_t parent_freq_hz, bool query);
uint32_t clk_div_set_freq(struct clk *clkp, uint32_t target_hz, uint32_t min_hz, uint32_t max_hz,
			  bool query, bool *changed);
uint32_t clk_div_get_freq(struct clk *clkp);
uint32_t clk_div_set_freq_static_parent(struct clk *clkp, uint32_t target_hz, uint32_t min_hz,
					uint32_t max_hz, bool query, bool *changed);

uint32_t clk_div_reg_go_get_div(struct clk *clkp);
bool clk_div_reg_go_set_div(struct clk *clkp, uint32_t d);
uint32_t clk_div_reg_get_div(struct clk *clkp);
bool clk_div_reg_set_div(struct clk *clkp, uint32_t d);

uint32_t clk_get_div(struct clk *clkp);
bool clk_set_div(struct clk *clkp, uint32_t d);
int32_t clk_div_init(struct clk *clkp);

#endif
