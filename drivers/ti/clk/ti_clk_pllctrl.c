/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI PLL Control Driver
 *
 * This driver provides low-level control for PLL enable/disable and lock
 * status monitoring. It manages PLL control registers including PLLEN
 * (PLL enable), PLLENSRC (enable source select), and PLLSTAT (lock status).
 * The driver also implements mux functionality for PLL bypass/lock-based
 * clock path selection.
 */

#include <ti_clk_pllctrl.h>
#include <ti_container_of.h>
#include <ti_io.h>
#define PLLCTRL_PLLCTRL			0x100U
#define PLLCTRL_PLLCTRL_PLLEN		BIT(0)
#define PLLCTRL_PLLCTRL_PLLENSRC	BIT(5)

#define PLLCTRL_PLLSTAT			0x13cU
#define PLLCTRL_PLLSTAT_LOCK		BIT(1)

static const struct ti_clk_parent *ti_clk_pllctrl_mux_get_parent(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_datap = clk_get_data(clkp);
	const struct ti_clk_data_mux *mux;
	const struct ti_clk_data_mux_reg *reg;
	const struct ti_clk_parent *parent = NULL;
	uint32_t reg_val;

	mux = container_of(clk_datap->data, const struct ti_clk_data_mux, data);
	reg = container_of(mux, const struct ti_clk_data_mux_reg, data_mux);

	reg_val = (uint32_t)readl(reg->reg + PLLCTRL_PLLCTRL);
	if ((reg_val & PLLCTRL_PLLCTRL_PLLENSRC) != 0U) {
		/* When set, mux is controlled by lock state of PLL */
		reg_val = (uint32_t)readl(reg->reg + PLLCTRL_PLLSTAT);
		if ((reg_val & PLLCTRL_PLLSTAT_LOCK) != 0U) {
			/* PLL is locked */
			parent = &mux->parents[1];
		} else {
			/* PLL is in bypass */
			parent = &mux->parents[0];
		}
	} else {
		/* When cleaned, mux is controlled by PLLEN bit */
		if ((reg_val & PLLCTRL_PLLCTRL_PLLEN) != 0U) {
			/* Use pll clock */
			parent = &mux->parents[1];
		} else {
			/* Use bypass clock */
			parent = &mux->parents[0];
		}
	}

	/* If div is 0, parent clock is not connected */
	if ((parent != NULL) && (parent->div == 0U)) {
		parent = NULL;
	}

	return parent;
}

const struct ti_clk_drv_mux ti_clk_drv_pllctrl_mux_reg_ro = {
	.get_parent = ti_clk_pllctrl_mux_get_parent,
};
