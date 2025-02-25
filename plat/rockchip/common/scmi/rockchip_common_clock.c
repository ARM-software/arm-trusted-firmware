// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#include <assert.h>
#include <errno.h>

#include <drivers/scmi.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include <plat_private.h>
#include <scmi_clock.h>

#define MUX_ADDR_INFO			0
#define MUX_SHIFT_INFO			1
#define MUX_WIDTH_INFO			2
#define DIV_ADDR_INFO			3
#define DIV_SHIFT_INFO			4
#define DIV_WIDTH_INFO			5
#define GATE_ADDR_INFO			6
#define GATE_SHIFT_INFO			7

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define abs(x) ({						\
		long ret;					\
		if (sizeof(x) == sizeof(long)) {		\
			long __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		} else {					\
			int __x = (x);				\
			ret = (__x < 0) ? -__x : __x;		\
		}						\
		ret;						\
	})

static unsigned long clk_scmi_common_get_parent_rate(rk_scmi_clock_t *clock,
						     int id)
{
	rk_scmi_clock_t *p_clock;

	if (clock->is_dynamic_prate != 0) {
		p_clock = rockchip_scmi_get_clock(0, clock->parent_table[id]);
		if (p_clock == NULL)
			return 0;
		if ((p_clock->clk_ops != NULL) && (p_clock->clk_ops->get_rate != NULL))
			return  p_clock->clk_ops->get_rate(p_clock);
		else
			return 0;
	} else {
		return clock->parent_table[id];
	}
}

unsigned long clk_scmi_common_get_rate(rk_scmi_clock_t *clock)
{
	unsigned long parent_rate, sel, div;

	sel = mmio_read_32(clock->info[MUX_ADDR_INFO]) >>
	      clock->info[MUX_SHIFT_INFO];
	sel = sel & (BIT(clock->info[MUX_WIDTH_INFO]) - 1);
	div = mmio_read_32(clock->info[DIV_ADDR_INFO]) >>
	      clock->info[DIV_SHIFT_INFO];
	div = div & (BIT(clock->info[DIV_WIDTH_INFO]) - 1);
	parent_rate = clk_scmi_common_get_parent_rate(clock, sel);

	return parent_rate / (div + 1);
}

int clk_scmi_common_set_rate(rk_scmi_clock_t *clock, unsigned long rate)
{
	unsigned long parent_rate, now, best_rate = 0;
	int i = 0, sel_mask, div_mask, best_sel = 0, best_div = 0, div;

	if ((rate == 0) ||
	    (clock->info[MUX_WIDTH_INFO] == 0 && clock->info[DIV_WIDTH_INFO] == 0))
		return SCMI_INVALID_PARAMETERS;

	sel_mask = BIT(clock->info[MUX_WIDTH_INFO]) - 1;
	div_mask = BIT(clock->info[DIV_WIDTH_INFO]) - 1;
	if (clock->info[MUX_WIDTH_INFO] == 0) {
		parent_rate = clk_scmi_common_get_parent_rate(clock, 0);
		div = DIV_ROUND_UP(parent_rate, rate);
		if (div > div_mask + 1)
			div = div_mask + 1;
		mmio_write_32(clock->info[DIV_ADDR_INFO],
			      BITS_WITH_WMASK(div - 1, div_mask,
					      clock->info[DIV_SHIFT_INFO]));
	} else if (clock->info[DIV_WIDTH_INFO] == 0) {
		for (i = 0; i <= sel_mask; i++) {
			parent_rate = clk_scmi_common_get_parent_rate(clock, i);
			now = parent_rate;
			if (abs(rate - now) < abs(rate - best_rate)) {
				best_rate = now;
				best_sel = i;
			}
		}
		if (best_rate == 0)
			best_sel = 0;
		mmio_write_32(clock->info[MUX_ADDR_INFO],
			      BITS_WITH_WMASK(best_sel, sel_mask,
					      clock->info[MUX_SHIFT_INFO]));
	} else {
		for (i = 0; i <= sel_mask; i++) {
			parent_rate = clk_scmi_common_get_parent_rate(clock, i);
			div = DIV_ROUND_UP(parent_rate, rate);
			if (div > div_mask + 1)
				div = div_mask + 1;
			now = parent_rate / div;
			if (abs(rate - now) < abs(rate - best_rate)) {
				best_rate = now;
				best_div = div;
				best_sel = i;
			}
		}
		if (best_rate == 0) {
			best_div = div_mask + 1;
			best_sel = 0;
		}

		mmio_write_32(clock->info[DIV_ADDR_INFO],
			      BITS_WITH_WMASK(div_mask, div_mask,
					      clock->info[DIV_SHIFT_INFO]));
		mmio_write_32(clock->info[MUX_ADDR_INFO],
			      BITS_WITH_WMASK(best_sel, sel_mask,
					      clock->info[MUX_SHIFT_INFO]));
		mmio_write_32(clock->info[DIV_ADDR_INFO],
			      BITS_WITH_WMASK(best_div - 1, div_mask,
					      clock->info[DIV_SHIFT_INFO]));
	}
	return 0;
}

int clk_scmi_common_set_status(rk_scmi_clock_t *clock, bool status)
{
	mmio_write_32(clock->info[GATE_ADDR_INFO],
		      BITS_WITH_WMASK(!status, 0x1U,
				      clock->info[GATE_SHIFT_INFO]));

	return 0;
}
