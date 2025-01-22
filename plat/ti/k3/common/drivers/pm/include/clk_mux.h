/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLK_MUX_H
#define CLK_MUX_H

#include <clk.h>

struct clk_data_mux {
	struct clk_drv_data	data;
	uint32_t			n;
	const struct clk_parent *parents;
};

struct clk_data_mux_reg {
	struct clk_data_mux	data_mux;
	uint32_t			reg;
	uint8_t			bit;
};

struct clk_drv_mux {
	struct clk_drv		drv;
	bool			(*set_parent)(struct clk *clkp, uint8_t p);
	const struct clk_parent * (*get_parent)(struct clk *clkp);
};

extern const struct clk_drv_mux clk_drv_mux_reg_ro;
extern const struct clk_drv_mux clk_drv_mux_reg;

const struct clk_parent *clk_get_parent(struct clk *clkp);
bool clk_set_parent(struct clk *clkp, uint8_t new_parent);

#endif
