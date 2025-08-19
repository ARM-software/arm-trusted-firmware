/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Clock Management Framework Core
 *
 * This is the core clock framework that provides the central clock tree
 * management, including reference counting, parent-child relationships,
 * frequency propagation, and clock state management. It implements the
 * main clock API functions for get/put, set frequency, query frequency,
 * and state control that are used by device drivers and power management.
 */

#include <limits.h>
#include <stddef.h>
#include <errno.h>

#include <common/debug.h>

#include <ti_clk.h>
#include <ti_clk_mux.h>
#include <ti_device.h>
#include <ti_container_of.h>

#define CLK_DATA_FLAG_BLOCK_FREQ_CHANGE		BIT(3)
#define CLK_ID_NONE ((clk_idx_t) (0xFFFFFFFFU))

/** Maximum number of times to walk the clock tree in LPM handlers */
#define LPM_CLK_MAX_TRIES		10

uint32_t ti_clk_value_set_freq(struct ti_clk *clkp, uint32_t target_hz,
			    uint32_t min_hz __maybe_unused,
			    uint32_t max_hz __maybe_unused,
			    bool query, bool *changed)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);

	if (!query) {
		*changed = true;
		soc_clock_values[clk_data_p->freq_idx] = target_hz;
	}

	return target_hz;
}

uint32_t ti_clk_value_get_freq(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);

	return soc_clock_values[clk_data_p->freq_idx];
}

uint32_t ti_clk_get_parent_freq(struct ti_clk *clkp)
{
	const struct ti_clk_parent *p = NULL;
	struct ti_clk *parent_clk;

	if (clkp != NULL) {
		p = ti_clk_mux_get_parent(clkp);
	}

	if (p && p->div) {
		parent_clk = clk_lookup((clk_idx_t) p->clk);
		if (parent_clk != NULL) {
			return ti_clk_get_freq(parent_clk) / p->div;
		}
	}

	return 0;
}

bool ti_clk_notify_freq(struct ti_clk *clkp, uint32_t parent_freq_hz, bool query)
{
	const struct ti_clk_data *clk_data_p;
	bool ret = true;

	if (clkp->freq_change_block_count != 0U) {
		ret = false;
	}

	clk_data_p = clk_get_data(clkp);

	if (ret) {
		const struct ti_clk_range *range;

		range = clk_get_range(clk_data_p->range_idx);
		if ((parent_freq_hz < range->min_hz) ||
		    (parent_freq_hz > range->max_hz)) {
			ret = false;
		}
	}

	if (ret) {
		if (clk_data_p->drv->notify_freq != NULL) {
			ret = clk_data_p->drv->notify_freq(clkp, parent_freq_hz,
							   query);
		} else {
			ret = clk_notify_children_freq(clkp, parent_freq_hz,
						       query);
		}
	}

	if (ret && !query) {
		clkp->flags &= (uint8_t) ~TI_CLK_FLAG_CACHED;
	}

	return ret;
}

bool clk_notify_sibling_freq(struct ti_clk *clkpp, struct ti_clk *parent,
			     uint32_t parent_freq, bool query)
{
	struct ti_clk *clkp = clkpp;
	clk_idx_t i;
	clk_idx_t pid = clk_id(parent);
	clk_idx_t id = clkp ? clk_id(clkp) : CLK_ID_NONE;
	bool status = true;

	/* We must unfortunately walk clock list to find children */
	for (i = 0U; i < soc_clock_count; i++) {
		const struct ti_clk_parent *p;

		/* Skip over clk */
		if (i == id) {
			continue;
		}

		clkp = clk_lookup(i);
		if (!clkp || ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U)) {
			continue;
		}

		p = ti_clk_mux_get_parent(clkp);
		if (!p || (p->clk != pid)) {
			continue;
		}

		if (!ti_clk_notify_freq(clkp, parent_freq / p->div, query)) {
			status = false;
			break;
		}
	}

	return status;
}

bool clk_notify_children_freq(struct ti_clk *parent, uint32_t parent_freq,
			      bool query)
{
	/*
	 * Reuse sibling walk function, it just avoids whatever clock is in
	 * clk and matches whatever clocks are children of parent.
	 */
	return clk_notify_sibling_freq(parent, parent, parent_freq, query);
}

uint32_t ti_clk_generic_set_freq_parent(struct ti_clk *clkp, struct ti_clk *parent,
				     uint32_t target_hz, uint32_t min_hz,
				     uint32_t max_hz, bool query,
				     bool *changed, uint32_t div)
{
	uint32_t new_target, new_min, new_max;
	uint32_t new_parent_freq = 0;

	/* Make sure target fits within out clock frequency type */
	if (((uint32_t) ULONG_MAX / div) < min_hz) {
		return 0;
	}

	new_min = min_hz * div;
	new_target = target_hz * div;
	new_max = max_hz * div;

	if (new_min < min_hz) {
		return 0;
	}

	/* Cap overflow in target and max */
	if (new_target < new_min) {
		new_target = (uint32_t) ULONG_MAX;
	}

	if (new_max < new_target) {
		new_max = (uint32_t) ULONG_MAX;
	}

	new_parent_freq = clk_set_freq(parent, new_target,
				       new_min, new_max,
				       true, changed);

	if (new_parent_freq == 0U) {
		return 0;
	}

	/* Check that any siblings can handle the new freq */
	if (*changed && !(clk_notify_sibling_freq(clkp,
						  parent,
						  new_parent_freq,
						  true))) {
		return 0;
	}

	if (query) {
		return new_parent_freq / div;
	}

	/* Actually perform the frequency change */
	clk_set_freq(parent, new_target,
		     new_min, new_max,
		     false, changed);

	/* Notify new rate to siblings */
	if (*changed) {
		clk_notify_sibling_freq(clkp,
					parent,
					new_parent_freq,
					false);
	}

	return new_parent_freq / div;
}

static uint32_t clk_generic_set_freq(struct ti_clk *clkp,
				     uint32_t target_hz,
				     uint32_t min_hz,
				     uint32_t max_hz, bool query,
				     bool *changed)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);
	const struct ti_clk_parent *p = ti_clk_mux_get_parent(clkp);

	*changed = false;

	if (p && (clk_data_p->flags & TI_CLK_DATA_FLAG_MODIFY_PARENT_FREQ)) {
		struct ti_clk *parent;

		parent = clk_lookup((clk_idx_t) p->clk);

		if (parent != NULL) {
			return ti_clk_generic_set_freq_parent(clkp, parent,
							  target_hz,
							  min_hz, max_hz,
							  query, changed,
							  p->div);
		}
	} else {
		uint32_t freq = ti_clk_get_freq(clkp);

		if ((freq >= min_hz) && (freq <= max_hz)) {
			return freq;
		}
	}

	return 0;
}

uint32_t clk_set_freq(struct ti_clk *clkp, uint32_t target_hz,
		      uint32_t min_hz, uint32_t max_hz, bool query,
		      bool *changed)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);
	uint32_t ret;

	*changed = false;

	if ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U) {
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

	if (ret) {
		VERBOSE("CLOCK_SET_RATE: clk_id=%d freq=%d\n", clk_id(clkp), target_hz);
	} else {
		VERBOSE("ACTION_FAIL CLOCK_SET_RATE: clk_id=%d freq=%d\n", clk_id(clkp), target_hz);
	}

	return ret;
}

uint32_t ti_clk_get_freq(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);
	uint32_t ret;

	if ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U) {
		ret = 0U;
	} else if (clk_data_p->drv->get_freq != NULL) {
		ret = clk_data_p->drv->get_freq(clkp);
	} else {
		ret = ti_clk_get_parent_freq(clkp);
	}

	return ret;
}

uint32_t ti_clk_get_state(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);
	uint32_t ret = TI_CLK_HW_STATE_DISABLED;

	if ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U) {
		ret = TI_CLK_HW_STATE_DISABLED;
	} else if (clk_data_p->drv->get_state != NULL) {
		ret = clk_data_p->drv->get_state(clkp);
	} else {
		const struct ti_clk_parent *p;

		p = ti_clk_mux_get_parent(clkp);

		if (p != NULL) {
			struct ti_clk *clkp_parent;

			clkp_parent = clk_lookup((clk_idx_t) p->clk);

			if (clkp_parent != NULL) {
				ret = ti_clk_get_state(clkp_parent);
			}
		}
	}

	return ret;
}

bool ti_clk_set_state(struct ti_clk *clkp, bool enable)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);
	bool ret = true;

	if ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U) {
		/* defer action */
		return ret;
	}

	if (clk_data_p->drv->set_state != NULL) {
		ret = clk_data_p->drv->set_state(clkp, enable);
	}

	return ret;
}

bool ti_clk_get(struct ti_clk *clkp)
{
	bool ret = true;

	if (clkp->ref_count == 0U) {
		const struct ti_clk_parent *p;
		struct ti_clk *clkp_parent = NULL;

		p = ti_clk_mux_get_parent(clkp);
		if (p != NULL) {
			clkp_parent = clk_lookup((clk_idx_t) p->clk);
		}

		if (clkp_parent != NULL) {
			ret = ti_clk_get(clkp_parent);
		}

		if (ret) {
			ret = ti_clk_set_state(clkp, true);
			if (!ret && clkp_parent) {
				VERBOSE("ACTION FAIL\n");
				ti_clk_put(clkp_parent);
			}
			VERBOSE("CLOCK_ENABLE: clk_id=%d\n", clk_id(clkp));
		}
	}

	if (ret) {
		clkp->ref_count++;
	}

	return ret;
}

void ti_clk_put(struct ti_clk *clkp)
{
	if (--clkp->ref_count == 0U) {
		const struct ti_clk_parent *p;

		p = ti_clk_mux_get_parent(clkp);
		ti_clk_set_state(clkp, false);
		VERBOSE("CLOCK_DISABLE: clk_id=%d\n", clk_id(clkp));
		if (p != NULL) {
			struct ti_clk *clkp_parent;

			clkp_parent = clk_lookup((clk_idx_t) p->clk);
			if (clkp_parent != NULL) {
				ti_clk_put(clkp_parent);
			}
		}
	}
}


void ti_clk_freq_change_allow(struct ti_clk *clkp)
{
	clkp->freq_change_block_count--;
}

void ti_clk_freq_change_block(struct ti_clk *clkp)
{
	clkp->freq_change_block_count++;
}

static int32_t clk_register_clock(struct ti_clk *clkp, const struct ti_clk_data *clk_data_p)
{
	struct ti_clk *clkp_parent = NULL;
	const struct ti_clk_parent *p;
	int32_t ret;

	p = ti_clk_mux_get_parent(clkp);
	if (p != NULL) {
		clkp_parent = clk_lookup((clk_idx_t) p->clk);
	}
	if ((clkp_parent != NULL) && ((clkp_parent->flags & TI_CLK_FLAG_INITIALIZED) == 0U)) {
		return -EAGAIN;
	}

	if (clk_data_p->drv->init != NULL) {
		ret = clk_data_p->drv->init(clkp);
		if (ret != 0) {
			return ret;
		}
	}

	clkp->flags |= TI_CLK_FLAG_INITIALIZED;
	if (clkp->ref_count != 0U) {
		if (!ti_clk_set_state(clkp, true)) {
			VERBOSE("ACTION FAIL\n");
		}
		VERBOSE("CLOCK_ENABLE: clk_id=%d\n", clk_id(clkp));
	}
	if (clk_data_p->drv->get_state != NULL) {
		if (clk_data_p->drv->get_state(clkp) != TI_CLK_HW_STATE_DISABLED) {
			clkp->flags |= TI_CLK_FLAG_PWR_UP_EN;
		}
	}

	return 0;
}

void ti_clk_drop_pwr_up_en(void)
{
	clk_idx_t i;

	for (i = 0U; i < soc_clock_count; i++) {
		if ((soc_clocks[i].flags & TI_CLK_FLAG_PWR_UP_EN) != 0U) {
			ti_clk_put(&soc_clocks[i]);
			soc_clocks[i].flags &= (uint8_t) ~TI_CLK_FLAG_PWR_UP_EN;
		}
	}
}

/**
 * @brief Save clock state during suspend
 *
 * Calls the clock driver's suspend_save handler to save the clock's current
 * state before entering low power mode. Marks the clock as suspended to avoid
 * duplicate operations.
 *
 * @param clkp Clock to save state for
 *
 * @return 0 on success, error code otherwise
 */
static int32_t ti_clk_suspend_save(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);
	int32_t ret = 0;

	if (clk_data_p != NULL) {
		if ((clkp->flags & TI_CLK_FLAG_SUSPENDED) == 0U) {
			if (clk_data_p->drv->suspend_save != NULL) {
				ret = clk_data_p->drv->suspend_save(clkp);
				/* Mark clock as suspended to avoid duplicate operations */
				clkp->flags |= TI_CLK_FLAG_SUSPENDED;
			} else {
				/* Mark clock as suspended if no handler is provided */
				clkp->flags |= TI_CLK_FLAG_SUSPENDED;
				ret = 0;
			}
		}
	} else {
		ret = 0;
	}

	return ret;
}

/**
 * @brief Restore clock state during resume
 *
 * Calls the clock driver's resume_restore handler to restore the clock's
 * state after exiting low power mode. Ensures parent clocks are restored
 * first before restoring child clocks. Clears the suspended flag after
 * successful restoration.
 *
 * @param clkp Clock to restore state for
 *
 * @return 0 on success, -EINVAL if parent not yet restored, error code otherwise
 */
static int32_t ti_clk_resume_restore(struct ti_clk *clkp)
{
	const struct ti_clk_data *clk_data_p = clk_get_data(clkp);
	const struct ti_clk_parent *p = NULL;
	struct ti_clk *parent_clk = NULL;
	int32_t ret = 0;

	p = ti_clk_mux_get_parent(clkp);
	if (p != NULL) {
		parent_clk = clk_lookup((clk_idx_t)p->clk);
	}

	if (parent_clk != NULL) {
		/* If parent is still suspended, defer until it has resumed. */
		if ((parent_clk->flags & TI_CLK_FLAG_SUSPENDED) == TI_CLK_FLAG_SUSPENDED) {
			ret = -EINVAL;
		}
	}

	if ((ret != -EINVAL) && ((clkp->flags & TI_CLK_FLAG_SUSPENDED) == TI_CLK_FLAG_SUSPENDED)) {
		if (clk_data_p->drv->resume_restore != NULL) {
			ret = clk_data_p->drv->resume_restore(clkp);
			/* Clear suspended flag to avoid duplicate operations */
			clkp->flags &= ~TI_CLK_FLAG_SUSPENDED;
		} else {
			/* Mark clock as resumed if no handler is provided */
			clkp->flags &= ~TI_CLK_FLAG_SUSPENDED;
			ret = 0;
		}
	}

	return ret;
}

int32_t ti_clks_suspend(void)
{
	uint32_t i;
	int32_t ret = 0;
	uint32_t clock_count = soc_clock_count;
	uint8_t max_tries = LPM_CLK_MAX_TRIES;
	bool done, error;

	do {
		done = true;
		error = false;

		for (i = 1u; i < clock_count; i++) {
			struct ti_clk *clkp = soc_clocks + i;

			ret = ti_clk_suspend_save(clkp);
			if (ret == -EINVAL) {
				done = false;
			} else if (ret != 0) {
				error = true;
			} else {
				/* Do nothing */
			}
		}

		/* Avoid getting stuck forever, bound the number of loops */
		max_tries--;
	} while (!done && !error && (max_tries != 0U));

	if (max_tries == 0U) {
		ret = -ETIMEDOUT;
	} else {
		ret = 0;
	}

	return ret;
}

int32_t ti_clks_resume(void)
{
	uint32_t i;
	int32_t ret = 0;
	uint32_t clock_count = soc_clock_count;
	uint8_t max_tries = LPM_CLK_MAX_TRIES;
	bool done, error;

	do {
		done = true;
		error = false;

		for (i = 1u; i < clock_count; i++) {
			struct ti_clk *clkp = soc_clocks + i;

			ret = ti_clk_resume_restore(clkp);

			if (ret == -EINVAL) {
				done = false;
			} else if (ret != 0) {
				error = true;
			} else {
				/* Do nothing */
			}
		}

		/* Avoid getting stuck forever, bound the number of loops */
		max_tries--;
	} while (!done && !error && (max_tries != 0U));

	if (max_tries == 0U) {
		ret = -ETIMEDOUT;
	} else {
		ret = 0;
	}

	return ret;
}

int32_t ti_clk_init(void)
{
	bool progress;
	bool contents;
	bool enabled = true;
	int32_t last_error = 0;
	uint32_t i;
	uint32_t clock_count = soc_clock_count;

	for (i = 0U; i < soc_devgroup_count; i++) {
		devgrp_t devgrp;

		/* Translate compressed internal representation to bitfield */
		devgrp = (devgrp_t) BIT(i - 1U);

		/* First disabled devgroup, stop at this clock index */
		if (enabled && !ti_pm_devgroup_is_enabled(devgrp)) {
			clock_count = soc_devgroups[i].clk_idx;
			enabled = false;
		}
	}

	contents = false;
	progress = false;

	/* Loop through all the clocks to initialize them */
	for (i = 0U; i < clock_count; i++) {
		struct ti_clk *clkp = &soc_clocks[i];
		const struct ti_clk_data *clk_data_p = &soc_clock_data[i];
		int32_t curr;

		if (((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U) && (clk_data_p->drv != NULL)) {
			contents = true;
			curr = clk_register_clock(clkp, clk_data_p);
			if (curr != -EAGAIN) {
				progress = true;
				if (curr != 0) {
					last_error = curr;
				}
			}
		}
	}

	if (progress) {
		for (i = 0U; i < clock_count; i++) {
			if ((soc_clocks[i].flags & TI_CLK_FLAG_PWR_UP_EN) != 0U) {
				if (!ti_clk_get(&soc_clocks[i])) {
					/* ti_clk_get failed for one of the clocks */
					return -ENODEV;
				}
			}
		}
		return last_error;
	}

	if (contents) {
		/* We processed at least one clock but didn't make progress */
		return -EAGAIN;
	}

	/* We didn't process any clocks */
	return 0;
}
