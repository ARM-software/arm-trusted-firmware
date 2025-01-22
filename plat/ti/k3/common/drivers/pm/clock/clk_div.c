/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_div.h>
#include <clk_mux.h>
#include <lib/container_of.h>
#include <ilog.h>
#include <lib/io.h>

uint32_t clk_get_div(struct clk *clkp)
{
	uint32_t ret = 0U;
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_drv_div *divp;

	if (clk_datap->type != CLK_TYPE_DIV) {
		ret = 1U;
	} else {
		divp = container_of(clk_datap->drv, const struct clk_drv_div, drv);
		ret = divp->get_div(clkp);
	}
	return ret;
}

bool clk_div_notify_freq(struct clk *clkp, uint32_t parent_freq_hz,
			 bool query)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;
	const struct clk_drv_div *drv_div;
	uint32_t divp = clk_get_div(clkp);
	uint32_t i;
	bool found = false;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	drv_div = container_of(clk_datap->drv, const struct clk_drv_div, drv);

	/* Just find a frequency that works for all children */

	/* Try the current divisor and lower frequencies */
	for (i = divp; !found && (i < (data_div->n + 1UL)); i++) {
		if (!drv_div->valid_div || drv_div->valid_div(clkp, i)) {
			found = clk_notify_children_freq(clkp,
							 parent_freq_hz / i, true);
		}
	}

	/* Try larger frequencies */
	for (i = divp - 1U; !found && (i > 0UL); i--) {
		if (!drv_div->valid_div || drv_div->valid_div(clkp, i)) {
			found = clk_notify_children_freq(clkp,
							 parent_freq_hz / i, true);
		}
	}

	if (found && !query) {
		if (i > divp) {
			/* Frequency going down */
			drv_div->set_div(clkp, i);
		}
		clk_notify_children_freq(clkp, parent_freq_hz / i, false);
		if (i < divp) {
			/* Frequency going up */
			drv_div->set_div(clkp, i);
		}
	}

	return found;
}

static uint32_t clk_div_set_freq_dyn_parent(struct clk *clkp,
					    uint32_t target_hz, uint32_t min_hz, uint32_t max_hz,
					    bool query, bool *changed)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_parent *p = clk_get_parent(clkp);
	const struct clk_data_div *data_div;
	const struct clk_drv_div *drv_div;
	uint32_t old_div;
	uint32_t best_div = 0U;
	uint32_t best_parent_freq = 0U;
	uint32_t best_min_hz = 0U;
	uint32_t best_max_hz = 0U;
	uint32_t updated_min_hz = min_hz;
	uint32_t updated_max_hz = max_hz;
	bool best_changed = false;
	uint32_t min_delta = (uint32_t) UINT_MAX;
	uint32_t i;
	uint32_t ret;
	struct clk *parent = NULL;
	bool flag_break = false;


	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	drv_div = container_of(clk_datap->drv, const struct clk_drv_div, drv);

	/* p and clk_lookup verified by caller */
	if (p != NULL) {
		parent = clk_lookup((clk_idx_t) p->clk);
	}

	old_div = drv_div->get_div(clkp);

	for (i = 1U; (i <= data_div->n) && (p != NULL) && (min_delta != 0U); i++) {
		uint32_t new_target, new_min, new_max, new_parent_freq;
		uint32_t delta;
		uint32_t divider;
		bool new_target_overflow;

		if (drv_div->valid_div && !drv_div->valid_div(clkp, i)) {
			continue;
		}

		divider = i * p->div;

		/* Make sure target fits within out clock frequency type */
		if (((uint32_t) ULONG_MAX / divider) < min_hz) {
			continue;
		}

		new_min = updated_min_hz * divider;
		new_target = target_hz * divider;
		new_max = updated_max_hz * divider;

		/* If an overflow occurs in min, we are outside the range */
		if (new_min < updated_min_hz) {
			flag_break = true;
		}

		if (flag_break == false) {
			/* Cap overflow in target */
			if (new_target < target_hz) {
				if (best_div != 0U) {
					/*
					 * Already found a working combination, don't
					 * bother with target overflows.
					 */
					flag_break = true;
				} else {
					new_target = UINT_MAX;
					new_target_overflow = true;
				}
			} else {
				new_target_overflow = false;
			}
		}
		if (flag_break == true) {
			break;
		}

		/* Cap overflow in max */
		if (new_max < updated_max_hz) {
			new_max = (uint32_t) ULONG_MAX;
		}
		if (parent != NULL) {
			new_parent_freq = clk_set_freq(parent, new_target, new_min,
						       new_max, true, changed);
		} else {
			new_parent_freq = 0U;
		}

		if (new_parent_freq == 0U) {
			continue;
		}

		/* Check that any siblings can handle the new freq */
		if (*changed && !(clk_notify_sibling_freq(clkp, parent,
							  new_parent_freq,
							  true))) {
			continue;
		}

		if (new_target_overflow) {
			uint32_t new_parent_freq_div;
			/*
			 * The target frequency was capped to ULONG_MAX, our
			 * delta will be slightly less accurate.
			 */
			new_parent_freq_div = new_parent_freq / divider;
			if (new_parent_freq_div > target_hz) {
				delta = new_parent_freq_div - target_hz;
			} else {
				delta = target_hz - new_parent_freq_div;
			}
		} else {
			if (new_parent_freq > new_target) {
				delta = new_parent_freq - new_target;
			} else {
				delta = new_target - new_parent_freq;
			}
			delta /= divider;
		}
		if (delta >= min_delta) {
			continue;
		}

		min_delta = delta;
		best_div = i;
		best_changed = *changed;
		best_parent_freq = new_parent_freq;
		best_min_hz = new_min;
		best_max_hz = new_max;

		/*
		 * Tighten min/max to decrease search space.
		 * Any new frequency must be an improvement by at least 1Hz.
		 * Note that we stop searching when min_delta reaches zero.
		 * Ensure that subtraction for min and addition for max do
		 * not overflow.
		 */
		if (min_delta != 0U) {
			uint32_t hz;

			hz = target_hz - (min_delta - 1U);
			if ((hz <= target_hz) && (updated_min_hz < hz)) {
				updated_min_hz = hz;
			}

			hz = target_hz + (min_delta - 1U);
			if ((hz >= target_hz) && (updated_max_hz > hz)) {
				updated_max_hz = hz;
			}
		}
	}

	if ((best_div != 0U) && !query) {
		/* Actually program out parents */
		if (best_changed) {
			clk_set_freq(parent, best_parent_freq, best_min_hz,
				     best_max_hz, false, changed);
		}

		/* Actually program our own register */
		if (old_div != best_div) {
			drv_div->set_div(clkp, best_div);
		}

		/* Assign new rate to siblings */
		if (best_changed) {
			clk_notify_sibling_freq(clkp, parent,
						best_parent_freq, false);
		}

		*changed = best_changed || (old_div != best_div);
	}

	if (best_div != 0U) {
		ret = best_parent_freq / (best_div * p->div);
	} else {
		ret = 0U;
	}

	return ret;
}

uint32_t clk_div_set_freq_static_parent(
	struct clk	*clkp,
	uint32_t		target_hz,
	uint32_t		min_hz,
	uint32_t		max_hz,
	bool		query,
	bool		*changed
	__maybe_unused)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;
	const struct clk_drv_div *drv_div;
	uint32_t parent_freq_hz = clk_get_parent_freq(clkp);
	uint32_t div0, div1;
	bool div0_ok, div1_ok;
	uint32_t div0_delta = 0, div1_delta;
	uint32_t div0_hz, div1_hz;
	uint32_t n;


	/* Calculate 2 best potential frequencies */
	div0 = parent_freq_hz / target_hz;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	drv_div = container_of(clk_datap->drv, const struct clk_drv_div, drv);

	n = data_div->n;

	/*
	 * Prevent out-of-bounds divider value. Rest of the code in the
	 * function will check if the resulting divider value is within
	 * the allowable min/max range.
	 */
	if (div0 > (n - 1U)) {
		div0 = n - 1U;
	}

	div1 = div0 + 1U;

	if (drv_div->valid_div != NULL) {
		for (; (div0 > 0UL) && !drv_div->valid_div(clkp, div0); div0--) {
			/* Step through loop until valid div is found */
		}

		for (; (div1 <= n) && !drv_div->valid_div(clkp, div1); div1++) {
			/* Step through loop until valid div is found */
		}
	}

	div0_ok = false;
	div0_hz = 0U;
	if (div0 != 0UL) {
		div0_hz = parent_freq_hz / div0;
		/* Check for in range */
		if (div0_hz <= max_hz) {
			div0_ok = true;
			div0_delta = div0_hz - target_hz;
		} else {
			div0_hz = 0U;
		}
	}

	div1_ok = false;
	div1_hz = 0U;
	if (div1 <= n) {
		div1_hz = parent_freq_hz / div1;
		if (div1_hz >= min_hz) {
			div1_ok = true;
			div1_delta = target_hz - div1_hz;
		} else {
			div1_hz = 0U;
		}
	}

	/* Make sure at least one of them is acceptable */
	if (div1_ok && (!div0_ok || (div1_delta < div0_delta))) {
		div0_ok = true;
		div0 = div1;
		div0_hz = div1_hz;
	}

	if (div0_ok && !query) {
		/* Actually program our own register */
		drv_div->set_div(clkp, div0);
	}

	return div0_hz;
}


uint32_t clk_div_set_freq(struct clk *clkp, uint32_t target_hz,
			  uint32_t min_hz, uint32_t max_hz,
			  bool query, bool *changed)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_parent *p = clk_get_parent(clkp);
	uint32_t ret = 0;

	*changed = false;

	if (!p || !clk_lookup((clk_idx_t) p->clk)) {
		/* Cannot function without parent */
	} else if ((clk_datap->flags & CLK_DATA_FLAG_MODIFY_PARENT_FREQ) != 0U) {
		ret = clk_div_set_freq_dyn_parent(clkp, target_hz, min_hz,
						  max_hz, query, changed);
	} else {
		ret = clk_div_set_freq_static_parent(clkp, target_hz, min_hz,
						     max_hz, query, changed);
	}

	return ret;
}

uint32_t clk_div_get_freq(struct clk *clkp)
{
	uint32_t parent_freq_hz = clk_get_parent_freq(clkp);


	return parent_freq_hz / clk_get_div(clkp);
}

int32_t clk_div_init(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;
	const struct clk_drv_div *drv_div;
	int32_t ret = SUCCESS;
	bool skip_hw_init = false;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	drv_div = container_of(clk_datap->drv, const struct clk_drv_div, drv);

	if ((clk_datap->flags & CLK_DATA_FLAG_NO_HW_REINIT) != 0U) {
		if (drv_div->get_div != NULL) {
			if (drv_div->get_div(clkp) != 1U) {
				skip_hw_init = true;
			}
		}
	}

	if (skip_hw_init == false) {
		if (data_div->default_div && drv_div->set_div) {
			if (!drv_div->set_div(clkp, data_div->default_div)) {
				ret = -EINVAL;
			}
		}
	}

	return ret;
}

uint32_t clk_div_reg_get_div(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;
	const struct clk_data_div_reg *data_reg;
	uint32_t v;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	data_reg = container_of(data_div, const struct clk_data_div_reg,
				data_div);

	/*
	 * Hack, temporarily return parent 0 for muxes without register
	 * assignments.
	 */
	if (data_reg->reg == 0U) {
		v = 1U;
	} else {
		uint32_t n = data_div->n;

		if (data_reg->start_at_1 == 0U) {
			n -= 1U;
		}
		v = readl(data_reg->reg) >> data_reg->bit;

		v &= (uint32_t) ((1U << ilog32(n)) - 1U);
		if (data_reg->start_at_1 == 0U) {
			v += 1U;
		}
	}

	return v;
}

bool clk_div_reg_set_div(struct clk *clkp, uint32_t d)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;
	const struct clk_data_div_reg *data_reg;
	const struct clk_drv_div *drv_div;
	bool ret = false;
	uint32_t n;
	uint32_t d_val_p = d;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	data_reg = container_of(data_div, const struct clk_data_div_reg,
				data_div);
	drv_div = container_of(clk_datap->drv, const struct clk_drv_div, drv);

	n = data_div->n;
	if ((d_val_p <= n) && (!drv_div->valid_div || drv_div->valid_div(clkp, d_val_p))) {
		uint32_t v;

		if (data_reg->start_at_1 == 0U) {
			d_val_p -= 1U;
			n -= 1U;
		}

		v = readl(data_reg->reg);
		v &= (uint32_t) ~(((1U << (uint32_t) ilog32(n)) - 1U) << data_reg->bit);
		v |= d_val_p << data_reg->bit;
		ti_clk_writel(v, (uint32_t) data_reg->reg);
		ret = true; /* HARD CODED */

	}
	return ret;
}

const struct clk_drv_div clk_drv_div_reg_ro = {
	.drv			= {
		.get_freq	= clk_div_get_freq,
	},
	.get_div		= clk_div_reg_get_div,
};

const struct clk_drv_div clk_drv_div_reg = {
	.drv			= {
		.notify_freq	= clk_div_notify_freq,
		.set_freq	= clk_div_set_freq,
		.get_freq	= clk_div_get_freq,
		.init		= clk_div_init,
	},
	.set_div		= clk_div_reg_set_div,
	.get_div		= clk_div_reg_get_div,
};

uint32_t clk_div_reg_go_get_div(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;
	const struct clk_data_div_reg_go *data_reg;
	uint32_t v;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	data_reg = container_of(data_div, const struct clk_data_div_reg_go,
				data_div);

	/*
	 * Hack, temporarily return parent 0 for muxes without register
	 * assignments.
	 */
	if (data_reg->reg == 0U) {
		v = 1;
	} else {
		uint32_t n = data_div->n;

		if (data_reg->start_at_1 == 0U) {
			n -= 1U;
		}
		v = readl(data_reg->reg) >> data_reg->bit;
		v &= (uint32_t) ((1U << ilog32(n)) - 1U);
		v += 1U;
	}

	return v;
}

bool clk_div_reg_go_set_div(struct clk *clkp, uint32_t d)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;
	const struct clk_data_div_reg_go *data_reg;
	const struct clk_drv_div *drv_div;
	bool ret = false;
	uint32_t n;
	uint32_t d_val_p = d;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	data_reg = container_of(data_div, const struct clk_data_div_reg_go,
				data_div);
	drv_div = container_of(clk_datap->drv, const struct clk_drv_div, drv);

	n = data_div->n;
	if ((d_val_p <= n) && (!drv_div->valid_div || drv_div->valid_div(clkp, d_val_p))) {
		uint32_t v;

		if (data_reg->start_at_1 == 0U) {
			d_val_p -= 1U;
			n -= 1U;
		}

		v = readl(data_reg->reg);
		v &= (uint32_t) ~(((1U << ilog32(n)) - 1U) << data_reg->bit);
		v &= (uint32_t) ~BIT(data_reg->go);
		v |= d_val_p << data_reg->bit;
		ti_clk_writel(v, data_reg->reg);
		ret = true;

		/* Go bit registers typically do not read back */
		v |= BIT(data_reg->go);
		ti_clk_writel(v, (uint32_t) data_reg->reg);
		v &= (uint32_t) ~BIT(data_reg->go);
		ti_clk_writel(v, (uint32_t) data_reg->reg);
	}

	return ret;
}

const struct clk_drv_div clk_drv_div_reg_go = {
	.drv			= {
		.notify_freq	= clk_div_notify_freq,
		.set_freq	= clk_div_set_freq,
		.get_freq	= clk_div_get_freq,
		.init		= clk_div_init,
	},
	.set_div		= clk_div_reg_go_set_div,
	.get_div		= clk_div_reg_go_get_div,
};

static uint32_t clk_div_fixed_get_div(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_div *data_div;

	data_div = container_of(clk_datap->data, const struct clk_data_div,
				data);
	return (uint32_t) data_div->n;
}

const struct clk_drv_div clk_drv_div_fixed = {
	.drv			= {
		.get_freq	= clk_div_get_freq,
	},
	.get_div		= clk_div_fixed_get_div,
};
