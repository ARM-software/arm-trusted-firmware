/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk.h>
#include <device.h>
#include <clk_mux.h>
#include <types/errno.h>
#include <limits.h>
#include <stddef.h>
#include <lib/container_of.h>
#include <lib/trace.h>
#include <common/debug.h>

/** Maximum number of times to walk the clock tree in LPM handlers */
#define LPM_CLK_MAX_TRIES		10

uint32_t clk_value_set_freq(struct clk *clkp, uint32_t target_hz,
			    uint32_t min_hz __maybe_unused,
			    uint32_t max_hz __maybe_unused,
			    bool query, bool *changed)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);

	if (!query) {
		*changed = true;
		soc_clock_values[clk_data_p->freq_idx] = target_hz;
	}

	return target_hz;
}

uint32_t clk_value_get_freq(struct clk *clkp)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);


	return soc_clock_values[clk_data_p->freq_idx];
}

static bool clk_input_notify_freq(struct clk	*clkp __maybe_unused,
				  uint32_t		parent_freq __attribute__(
					  (unused)),
				  bool	query __maybe_unused)
{
	return false;
}

static bool clk_input_set_state(struct clk	*clkp __maybe_unused,
				bool		enabled __attribute__(
					(unused)))
{
	return true;
}

static uint32_t clk_input_get_state(struct clk *clkp __maybe_unused)
{
	return CLK_HW_STATE_ENABLED;
}

const struct clk_drv clk_drv_input = {
	.set_freq	= clk_value_set_freq,
	.get_freq	= clk_value_get_freq,
	.notify_freq	= clk_input_notify_freq,
	.set_state	= clk_input_set_state,
	.get_state	= clk_input_get_state,
};

uint32_t clk_get_parent_freq(struct clk *clkp)
{
	const struct clk_parent *p = NULL;
	struct clk *parent_clk = NULL;
	uint32_t ret = 0;


	if (clkp != NULL) {
		p = clk_get_parent(clkp);
	}

	if (p && p->div) {
		parent_clk = clk_lookup((clk_idx_t) p->clk);
	}

	if (parent_clk != NULL) {
		ret = clk_get_freq(parent_clk) / p->div;
	}

	return ret;
}

bool clk_notify_freq(struct clk *clkp, uint32_t parent_freq_hz, bool query)
{
	const struct clk_data *clk_data_p;
	bool ret = true;

	if (clkp->freq_change_block_count != 0U) {
		ret = false;
	}

	clk_data_p = clk_get_data(clkp);

	if (ret) {
		const struct clk_range *range;

		range = clk_get_range(clk_data_p->range_idx);
		if ((parent_freq_hz < range->min_hz) ||
		    (parent_freq_hz > range->max_hz)) {
			ret = false;
		}
	}

	if (ret) {
		if (clk_data_p->drv->notify_freq != NULL) {
			ret = clk_data_p->drv->notify_freq(clkp, parent_freq_hz, query);
		} else {
			ret = clk_notify_children_freq(clkp, parent_freq_hz, query);
		}
	}

	if (ret && !query) {
		clkp->flags &= (uint8_t) ~CLK_FLAG_CACHED;
	}

	return ret;
}

bool clk_notify_sibling_freq(struct clk *clkpp, struct clk *parent,
			     uint32_t parent_freq, bool query)
{
	struct clk *clkp = clkpp;
	clk_idx_t i;
	clk_idx_t pid = clk_id(parent);
	clk_idx_t id = clkp ? clk_id(clkp) : CLK_ID_NONE;
	bool status = true;


	/* We must unfortunately walk clock list to find children */
	for (i = 0U; i < soc_clock_count; i++) {
		const struct clk_parent *p;

		/* Skip over clk */
		if (i == id) {
			continue;
		}

		clkp = clk_lookup(i);
		if (!clkp || ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U)) {
			continue;
		}

		p = clk_get_parent(clkp);
		if (!p || (p->clk != pid)) {
			continue;
		}

		if (!clk_notify_freq(clkp, parent_freq / p->div, query)) {
			status = false;
			break;
		}
	}

	return status;
}

bool clk_notify_children_freq(struct clk *parent, uint32_t parent_freq,
			      bool query)
{
	/*
	 * Reuse sibling walk function, it just avoids whatever clock is in
	 * clk and matches whatever clocks are children of parent.
	 */
	return clk_notify_sibling_freq(parent, parent, parent_freq, query);
}

uint32_t clk_generic_set_freq_parent(struct clk *clkp, struct clk *parent,
				     uint32_t target_hz, uint32_t min_hz,
				     uint32_t max_hz, bool query,
				     bool *changed, uint32_t d)
{
	uint32_t new_target, new_min, new_max;
	uint32_t new_parent_freq = 0;
	uint32_t ret;

	/* Make sure target fits within out clock frequency type */
	if (((uint32_t) ULONG_MAX / d) < min_hz) {
		ret = 0;
	} else {
		new_min = min_hz * d;
		new_target = target_hz * d;
		new_max = max_hz * d;

		if (new_min < min_hz) {
			ret = 0;
		} else {
			/* Cap overflow in target and max */
			if (new_target < new_min) {
				new_target = (uint32_t) ULONG_MAX;
			}

			if (new_max < new_target) {
				new_max = (uint32_t) ULONG_MAX;
			}

			new_parent_freq = clk_set_freq(parent, new_target, new_min, new_max,
						       true, changed);

			if (new_parent_freq == 0U) {
				ret = 0;
			} else {
				/* Check that any siblings can handle the new freq */
				if (*changed && !(clk_notify_sibling_freq(clkp, parent,
									  new_parent_freq, true))) {
					ret = 0;
				} else {
					if (query) {
						ret = new_parent_freq / d;
					} else {
						/* Actually perform the frequency change */
						clk_set_freq(parent, new_target, new_min, new_max,
							     false, changed);

						/* Notify new rate to siblings */
						if (*changed) {
							clk_notify_sibling_freq(clkp,
										parent,
										new_parent_freq,
										false);
						}

						ret = new_parent_freq / d;
					}
				}
			}
		}
	}

	return ret;
}

static uint32_t clk_generic_set_freq(struct clk *clkp,
				     uint32_t target_hz,
				     uint32_t min_hz,
				     uint32_t max_hz, bool query,
				     bool *changed)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	const struct clk_parent *p = clk_get_parent(clkp);
	uint32_t ret = 0;

	*changed = false;

	if (p && (clk_data_p->flags & CLK_DATA_FLAG_MODIFY_PARENT_FREQ)) {
		struct clk *parent;

		parent = clk_lookup((clk_idx_t) p->clk);

		if (parent != NULL) {
			ret = clk_generic_set_freq_parent(clkp, parent,
							  target_hz,
							  min_hz, max_hz,
							  query, changed,
							  p->div);
		}
	} else {
		uint32_t freq = clk_get_freq(clkp);

		if ((freq >= min_hz) && (freq <= max_hz)) {
			ret = freq;
		}
	}

	return ret;
}
#ifdef CONFIG_TRACE
static void clk_set_freq_trace(struct clk *clkp, uint32_t freq_hz, bool failed)
#else
	static void clk_set_freq_trace(struct clk *clkp __maybe_unused, uint32_t freq_hz,
				       bool failed)
#endif
{
	uint16_t trace_act = TRACE_PM_ACTION_CLOCK_SET_RATE;
	uint32_t val = freq_hz;
	uint32_t exp_val = 0;

	if (failed) {
		trace_act |= TRACE_PM_ACTION_FAIL;
	}

	while (val > TRACE_PM_VAL_MAX_FREQ) {
		val >>= 1;
		exp_val += 1U;
	}

	pm_trace(trace_act,
		 (val << TRACE_PM_VAL_CLOCK_VAL_SHIFT) |
		 (exp_val << TRACE_PM_VAL_CLOCK_EXP_SHIFT) |
		 ((uint32_t) (clk_id(clkp) << TRACE_PM_VAL_CLOCK_ID_SHIFT) &
		  TRACE_PM_VAL_CLOCK_ID_MASK));
}


uint32_t clk_set_freq(struct clk *clkp, uint32_t target_hz,
		      uint32_t min_hz, uint32_t max_hz, bool query,
		      bool *changed)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	uint32_t ret;

	*changed = false;

	if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
		ret = 0U;
	} else if ((clk_data_p->flags & CLK_DATA_FLAG_BLOCK_FREQ_CHANGE) != 0U) {
		ret = 0U;
	} else if (clk_data_p->drv->set_freq != NULL) {
		ret = clk_data_p->drv->set_freq(clkp, target_hz, min_hz,
						max_hz, query, changed);
	} else {
		ret = clk_generic_set_freq(clkp, target_hz, min_hz, max_hz,
					   query, changed);
	}

	clk_set_freq_trace(clkp, target_hz, ret == 0U);

	return ret;
}

uint32_t clk_get_freq(struct clk *clkp)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	uint32_t ret;

	if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
		ret = 0U;
	} else if (clk_data_p->drv->get_freq != NULL) {
		ret = clk_data_p->drv->get_freq(clkp);
	} else {
		ret = clk_get_parent_freq(clkp);
	}

	return ret;
}

uint32_t clk_get_state(struct clk *clkp)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	uint32_t ret = CLK_HW_STATE_DISABLED;

	if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
		ret = CLK_HW_STATE_DISABLED;
	} else if (clk_data_p->drv->get_state != NULL) {
		ret = clk_data_p->drv->get_state(clkp);
	} else {
		const struct clk_parent *p;

		p = clk_get_parent(clkp);

		if (p != NULL) {
			struct clk *clkp_parent;

			clkp_parent = clk_lookup((clk_idx_t) p->clk);

			if (clkp_parent != NULL) {
				ret = clk_get_state(clkp_parent);
			}
		}
	}

	return ret;
}

bool clk_set_state(struct clk *clkp, bool enable)
{
	const struct clk_data *clk_data_p = clk_get_data(clkp);
	bool ret;

	if (clk_data_p != NULL) {
		if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
			/* defer action */
			ret = true;
		} else if (clk_data_p->drv->set_state == NULL) {
			ret = true;
		} else {
			ret = clk_data_p->drv->set_state(clkp, enable);
		}
	} else {
		ret = true;
	}

	return ret;
}

bool clk_get(struct clk *clkp)
{
	bool ret = true;

	if (clkp->ref_count == 0U) {
		const struct clk_parent *p;
		struct clk *clkp_parent = NULL;

		p = clk_get_parent(clkp);
		if (p != NULL) {
			clkp_parent = clk_lookup((clk_idx_t) p->clk);
		}

		if (clkp_parent != NULL) {
			ret = clk_get(clkp_parent);
		}

		if (ret) {
			ret = clk_set_state(clkp, true);
			if (!ret && clkp_parent) {
				pm_trace(TRACE_PM_ACTION_CLOCK_ENABLE |
					 TRACE_PM_ACTION_FAIL, clk_id(clkp));
				clk_put(clkp_parent);
			} else {
				pm_trace(TRACE_PM_ACTION_CLOCK_ENABLE,
					 clk_id(clkp));
			}
		}
	}

	if (ret) {
		clkp->ref_count++;
	}

	return ret;
}

void clk_put(struct clk *clkp)
{
	if (--clkp->ref_count == 0U) {
		const struct clk_parent *p;

		p = clk_get_parent(clkp);
		clk_set_state(clkp, false);
		pm_trace(TRACE_PM_ACTION_CLOCK_DISABLE, clk_id(clkp));
		if (p != NULL) {
			struct clk *clkp_parent;

			clkp_parent = clk_lookup((clk_idx_t) p->clk);
			if (clkp_parent != NULL) {
				clk_put(clkp_parent);
			}
		}
	}
}

/* FIXME: Stop propagation at PLL and notify PLL */
void clk_ssc_allow(struct clk *clkp)
{
	if (--clkp->ssc_block_count == 0U) {
		const struct clk_parent *p;

		p = clk_get_parent(clkp);
		if (p != NULL) {
			struct clk *clkp_parent;

			clkp_parent = clk_lookup((clk_idx_t) p->clk);
			if (clkp_parent != NULL) {
				clk_ssc_allow(clkp_parent);
			}
		}
	}
}

void clk_ssc_block(struct clk *clkp)
{
	if (clkp->ssc_block_count++ == 0U) {
		const struct clk_parent *p;

		p = clk_get_parent(clkp);
		if (p != NULL) {
			struct clk *clkp_parent;

			clkp_parent = clk_lookup((clk_idx_t) p->clk);
			if (clkp_parent != NULL) {
				clk_ssc_block(clkp_parent);
			}
		}
	}
}

void clk_freq_change_allow(struct clk *clkp)
{
	clkp->freq_change_block_count--;
}

void clk_freq_change_block(struct clk *clkp)
{
	clkp->freq_change_block_count++;
}

static int32_t clk_register_clock(struct clk *clkp, const struct clk_data *clk_data_p)
{
	int32_t ret = SUCCESS;
	struct clk *clkp_parent = NULL;
	const struct clk_parent *p;

	p = clk_get_parent(clkp);
	if (p != NULL) {
		clkp_parent = clk_lookup((clk_idx_t) p->clk);
	}
	if ((clkp_parent != NULL) && ((clkp_parent->flags & CLK_FLAG_INITIALIZED) == 0U)) {
		ret = -EDEFER;
	}

	if ((ret == SUCCESS) && (clk_data_p->drv->init != NULL)) {
		ret = clk_data_p->drv->init(clkp);
	}

	if (ret == SUCCESS) {
		clkp->flags |= CLK_FLAG_INITIALIZED;
		if (clkp->ref_count != 0U) {
			if (!clk_set_state(clkp, true)) {
				pm_trace(TRACE_PM_ACTION_CLOCK_ENABLE |
					 TRACE_PM_ACTION_FAIL, clk_id(clkp));
			} else {
				pm_trace(TRACE_PM_ACTION_CLOCK_ENABLE,
					 clk_id(clkp));
			}
		}
		if (clk_data_p->drv->get_state != NULL) {
			if (clk_data_p->drv->get_state(clkp) != CLK_HW_STATE_DISABLED) {
				clkp->flags |= CLK_FLAG_PWR_UP_EN;
			}
		}
	}
	return ret;
}

void clk_drop_pwr_up_en(void)
{
	clk_idx_t i;

	for (i = 0U; i < soc_clock_count; i++) {
		if ((soc_clocks[i].flags & CLK_FLAG_PWR_UP_EN) != 0U) {
			clk_put(soc_clocks + i);
			soc_clocks[i].flags &= (uint8_t) ~CLK_FLAG_PWR_UP_EN;
		}
	}
}

int32_t clk_deinit_pm_devgrp(uint8_t pm_devgrp)
{
	int32_t ret = SUCCESS;
	uint32_t i;
	uint32_t clk_id_start;
	uint32_t clk_id_end = 0U;
	uint8_t pm_devgrp_val = pm_devgrp;

	clk_id_start = soc_devgroups[pm_devgrp_val].clk_idx;

	if (pm_devgrp_val >= soc_devgroup_count) {
		ret = -EINVAL;
	} else if (pm_devgrp_val == (soc_devgroup_count - 1U)) {
		/* Last devgrp's last clock id is the same as last of all clock ids */
		clk_id_end = soc_clock_count;
	} else {
		/* Chosen devgrp's last clock id is next devgrp's first clock id */

		/* Loop through all the devgrp till we find valid devgrp */
		while (pm_devgrp_val < soc_devgroup_count) {
			/* Check if next dev grp is valid devgrp*/
			if (soc_devgroups[pm_devgrp_val + 1U].clk_idx != 0U) {
				clk_id_end = soc_devgroups[pm_devgrp_val + 1U].clk_idx;
				break;
			}
			pm_devgrp_val = pm_devgrp_val + 1U;
		}

		/* If no valid devgrp is found,clock id is the same as last of all clock ids */
		if (pm_devgrp_val == soc_devgroup_count) {
			clk_id_end = soc_clock_count;
		}
	}

	/*
	 * Loop through all the clocks in selected device group.
	 * First pass make sure any clock with the PWR_UP_EN bit set gets
	 * a matching put call and the flag cleared. Clocks can be in this
	 * state if initialization for the given domain is in the deferred state.
	 */
	if (ret == SUCCESS) {
		for (i = clk_id_start; i < clk_id_end; i++) {
			struct clk *clkp = soc_clocks + i;

			/* Clear the power up flag */
			if ((clkp->flags & CLK_FLAG_PWR_UP_EN) != 0U) {
				clk_put(clkp);
				soc_clocks[i].flags &= (uint8_t) ~CLK_FLAG_PWR_UP_EN;
			}
		}
		/*
		 * Second pass clear the initialized flag and check that the ref_count
		 * is zero as expected.
		 */
		for (i = clk_id_start; i < clk_id_end; i++) {
			struct clk *clkp = soc_clocks + i;

			/* Clear the initialized flag */
			clkp->flags &= (uint8_t) ~CLK_FLAG_INITIALIZED;
		}
	}
	return ret;
}

int32_t clk_init(void)
{
	bool progress;
	bool contents;
	bool enabled = true;
	int32_t ret = SUCCESS;
	uint32_t i;
	uint32_t clock_count = soc_clock_count;

	for (i = 0U; i < soc_devgroup_count; i++) {
		devgrp_t devgrp;

		/* Translate compressed internal representation to bitfield */
		if (i == PM_DEVGRP_DMSC) {
			devgrp = DEVGRP_DMSC;
		} else {
			devgrp = (devgrp_t) BIT(i - 1U);
		}

		/* First disabled devgroup, stop at this clock index */
		if (enabled && !pm_devgroup_is_enabled(devgrp)) {
			clock_count = soc_devgroups[i].clk_idx;
			enabled = false;
		}
	}

	contents = false;
	progress = false;

	/* Loop through all the clocks to initialize them */
	for (i = 0U; i < clock_count; i++) {
		struct clk *clkp = soc_clocks + i;
		const struct clk_data *clk_data_p = soc_clock_data + i;
		int32_t curr;


		if (((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) && (clk_data_p->drv != NULL)) {
			contents = true;
			curr = clk_register_clock(clkp, clk_data_p);
			if (curr != -EDEFER) {
				progress = true;
				if (curr != SUCCESS) {
					ret = curr;
				}
			}
		}
	}

	if (progress) {
		for (i = 0U; i < clock_count; i++) {
			if ((soc_clocks[i].flags & CLK_FLAG_PWR_UP_EN) != 0U) {
				if (!clk_get(soc_clocks + i)) {
					/* clk_get failed for one of the clocks */
					ret = -EFAIL;
					break;
				}
			}
		}
	} else if (contents) {
		/* We processed at least one clock but didn't make progress */
		ret = -EDEFER;
	} else {
		/* We didn't process any clocks */
		ret = SUCCESS;
	}

	return ret;
}
