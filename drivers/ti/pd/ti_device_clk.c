/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Clock Management
 *
 * This module provides software support for managing clocks attached to
 * devices, including enable/disable, frequency scaling, parent selection,
 * and clock gating operations.
 */

#include <limits.h>
#include <stddef.h>

#include <ti_clk_mux.h>
#include <ti_device.h>
#include <ti_device_clk.h>

struct ti_dev_clk *ti_get_dev_clk(struct ti_device *dev, ti_dev_clk_idx_t idx)
{
	const struct ti_dev_data *data = ti_get_dev_data(dev);
	const struct ti_devgroup *devgrp = ti_dev_data_lookup_devgroup(data);
	struct ti_dev_clk *ret = NULL;

	if ((idx < data->n_clocks) && (devgrp != NULL)) {
		uint32_t offset = data->dev_clk_idx;

		if (ti_clk_id_valid(devgrp->dev_clk_data[offset + idx].clk)) {
			ret = &devgrp->dev_clk[offset + idx];
		}
	}

	return ret;
}

const struct ti_dev_clk_data *ti_get_dev_clk_data(struct ti_device *dev,
						  ti_dev_clk_idx_t idx)
{
	const struct ti_dev_data *data = ti_get_dev_data(dev);
	const struct ti_devgroup *devgrp = ti_dev_data_lookup_devgroup(data);
	const struct ti_dev_clk_data *ret = NULL;

	if ((idx < data->n_clocks) && (devgrp != NULL)) {
		uint32_t offset = data->dev_clk_idx;

		ret = &devgrp->dev_clk_data[offset + idx];
	}

	return ret;
}

struct ti_clk *ti_dev_get_clk(struct ti_device *dev, ti_dev_clk_idx_t idx)
{
	const struct ti_dev_clk_data *entry = ti_get_dev_clk_data(dev, idx);

	return entry ? ti_clk_lookup((ti_clk_idx_t) entry->clk) : NULL;
}

bool ti_device_clk_set_gated(struct ti_device *dev, ti_dev_clk_idx_t clk_idx, bool gated)
{
	const struct ti_dev_data *data = ti_get_dev_data(dev);
	const struct ti_devgroup *devgrp = ti_dev_data_lookup_devgroup(data);
	struct ti_dev_clk *dev_clkp = ti_get_dev_clk(dev, clk_idx);
	struct ti_clk *clkp = NULL;
	bool is_enabled = false;
	bool is_gated = false;
	bool ret = true;
	ti_clk_idx_t id;

	if ((dev_clkp == NULL) || (devgrp == NULL)) {
		ret = false;
	} else {
		is_gated = ((((uint32_t) (dev_clkp->flags) & TI_DEV_CLK_FLAG_DISABLE) > 0U) ?
			    true : false);
		if (is_gated != gated) {
			is_enabled = (dev->flags & TI_DEV_FLAG_ENABLED_MASK) != 0UL;
			id = (ti_clk_idx_t) devgrp->dev_clk_data[data->dev_clk_idx + clk_idx].clk;
			clkp = ti_clk_lookup(id);
			if (clkp == NULL) {
				/* Clock lookup failed */
				ret = false;
			}
		}
	}

	if ((clkp != NULL) && ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U)) {
		/* Clock not yet initialized (outside devgroup) */
		ret = false;
	} else if (clkp && gated) {
		dev_clkp->flags |= TI_DEV_CLK_FLAG_DISABLE;
		if (is_enabled) {
			ti_clk_put(clkp);
		}
	} else if (clkp != NULL) {
		dev_clkp->flags &= (uint8_t) ~TI_DEV_CLK_FLAG_DISABLE;
		if (is_enabled) {
			if (ti_clk_get(clkp)) {
				ret = true;
			} else {
				ret = false;
			}
		}
	} else {
		/* Do Nothing */
	}

	return ret;
}

bool ti_device_clk_get_sw_gated(struct ti_device *dev, ti_dev_clk_idx_t clk_idx)
{
	struct ti_dev_clk *dev_clkp = ti_get_dev_clk(dev, clk_idx);

	return (dev_clkp != NULL) && ((dev_clkp->flags & TI_DEV_CLK_FLAG_DISABLE) != 0U);
}

bool ti_device_clk_set_parent(struct ti_device *dev, ti_dev_clk_idx_t clk_idx,
			      ti_dev_clk_idx_t parent_idx)
{
	struct ti_clk *clkp;
	const struct ti_dev_clk_data *clock_data;
	const struct ti_dev_clk_data *parent_data = NULL;
	uint32_t depth;
	bool ret = true;

	clkp = ti_dev_get_clk(dev, clk_idx);
	clock_data = ti_get_dev_clk_data(dev, clk_idx);
	if ((clkp == NULL) || (clock_data == NULL)) {
		ret = false;
	}

	if (ret && (clock_data->type != TI_DEV_CLK_TABLE_TYPE_MUX)) {
		ret = false;
	}

	if (ret) {
		parent_data = ti_get_dev_clk_data(dev, parent_idx);
		if (parent_data == NULL) {
			ret = false;
		}
	}

	if (ret && (parent_data->type != TI_DEV_CLK_TABLE_TYPE_PARENT)) {
		ret = false;
	}

	/* Make sure it's within this clock muxes parents */
	if (ret && ((parent_idx - clk_idx) > clock_data->idx)) {
		ret = false;
	}

	depth = soc_clock_count;
	while (ret && (clkp != NULL) && (clkp->type != TI_CLK_TYPE_MUX) && (depth > 0U)) {
		const struct ti_clk_parent *p;

		depth--;
		p = ti_clk_mux_get_parent(clkp);
		if (p != NULL) {
			clkp = ti_clk_lookup((ti_clk_idx_t) p->clk);
		} else {
			clkp = NULL;
		}
	}

	if (depth == 0U) {
		/* Clock tree cycle detected or depth exceeded */
		ret = false;
	}

	if (clkp == NULL) {
		ret = false;
	}

	if (ret) {
		ret = ti_clk_mux_set_parent(clkp, parent_data->idx);
	}

	return ret;
}

ti_dev_clk_idx_t ti_device_clk_get_parent(struct ti_device *dev, ti_dev_clk_idx_t clk_idx)
{
	const struct ti_dev_data *data = ti_get_dev_data(dev);
	const struct ti_devgroup *devgroup_ptr = ti_dev_data_lookup_devgroup(data);
	const struct ti_clk_parent *p = NULL;
	struct ti_clk *clkp;
	const struct ti_dev_clk_data *clock_data;
	uint32_t depth;
	uint32_t offset;
	ti_dev_clk_idx_t i;

	clkp = ti_dev_get_clk(dev, clk_idx);
	clock_data = ti_get_dev_clk_data(dev, clk_idx);
	if ((clkp == NULL) || (clock_data == NULL) || (devgroup_ptr == NULL)) {
		return TI_DEV_CLK_ID_NONE;
	}

	if (clock_data->type != TI_DEV_CLK_TABLE_TYPE_MUX) {
		return TI_DEV_CLK_ID_NONE;
	}

	depth = soc_clock_count;
	while ((clkp != NULL) && (clkp->type != TI_CLK_TYPE_MUX) && (depth > 0U)) {
		depth--;
		p = ti_clk_mux_get_parent(clkp);
		if (p != NULL) {
			clkp = ti_clk_lookup((ti_clk_idx_t) p->clk);
		} else {
			clkp = NULL;
		}
	}

	if (depth == 0U) {
		/* Clock tree cycle detected or depth exceeded */
		return TI_DEV_CLK_ID_NONE;
	}

	if (clkp == NULL) {
		return TI_DEV_CLK_ID_NONE;
	}

	p = ti_clk_mux_get_parent(clkp);
	if (p == NULL) {
		return TI_DEV_CLK_ID_NONE;
	}

	offset = data->dev_clk_idx;
	for (i = 0U; i < clock_data->idx; i++) {
		if (devgroup_ptr->dev_clk_data[offset + i + clk_idx + 1U].clk == p->clk) {
			return i + clk_idx + 1U;
		}
	}

	return TI_DEV_CLK_ID_NONE;
}

ti_dev_clk_idx_t ti_device_clk_get_num_parents(struct ti_device *dev,
					       ti_dev_clk_idx_t clk_idx)
{
	struct ti_clk *clkp;
	const struct ti_dev_clk_data *clock_data;
	ti_dev_clk_idx_t ret;

	clkp = ti_dev_get_clk(dev, clk_idx);
	clock_data = ti_get_dev_clk_data(dev, clk_idx);
	if ((clkp == NULL) || (clock_data == NULL)) {
		ret = TI_DEV_CLK_ID_NONE;
	} else if (clock_data->type != TI_DEV_CLK_TABLE_TYPE_MUX) {
		if (ti_clk_mux_get_parent(clkp) != NULL) {
			ret = 1U;
		} else {
			ret = 0U;
		}
	} else {
		ret = (ti_dev_clk_idx_t) clock_data->idx;
		/*
		 * Encode both total parents and reserved count:
		 * Bits 0-7: total parent count (including reserved)
		 * Bits 8-15: reserved parent count
		 */
		ret = (ti_dev_clk_idx_t) ((clock_data->n_reserved_parents << 8) | clock_data->idx);
		VERBOSE("%s total=%d reserved=%d encoded=0x%x\n", __func__,
			clock_data->idx, clock_data->n_reserved_parents, ret);
	}

	return ret;
}

/**
 * ti_dev_clk_set_freq() - Set the frequency of a device's clock.
 * @dev: The device that the clock is connected to.
 * @clk_idx: The index of the clock on this device.
 * @min_freq_hz: The minimum acceptable frequency in Hz.
 * @target_freq_hz: The target frequency in Hz.
 * @max_freq_hz: The maximum acceptable frequency in Hz.
 *
 * Locates the correct clock and calls the internal clock API set frequency
 * function. If the clock is a mux type, the request is sent to the parent
 * to avoid switching the mux.
 *
 * Return: The actual frequency set, or 0 if no frequency could be found
 * within the limits.
 */
static uint32_t ti_dev_clk_set_freq(struct ti_device *dev, ti_dev_clk_idx_t clk_idx,
				    uint32_t min_freq_hz, uint32_t target_freq_hz,
				    uint32_t max_freq_hz)
{
	const struct ti_dev_data *data = ti_get_dev_data(dev);
	const struct ti_devgroup *devgroup_ptr = ti_dev_data_lookup_devgroup(data);
	const struct ti_dev_clk_data *clock_data;
	struct ti_clk *parent = NULL;
	uint32_t div_var = 1U;
	uint32_t ret_freq = 0U;
	ti_dev_clk_idx_t clk_idx_val = clk_idx;
	bool changed;

	if (devgroup_ptr == NULL) {
		return 0U;
	}

	clock_data = ti_get_dev_clk_data(dev, clk_idx_val);
	if (clock_data == NULL) {
		/* Invalid clock idx */
		return 0U;
	}

	parent = ti_dev_get_clk(dev, clk_idx_val);
	if (parent == NULL) {
		/* Parent not present */
		return 0U;
	}

	/* Assign div based on selected clock */
	div_var = clock_data->div;

	if ((parent->type == TI_CLK_TYPE_MUX) &&
	    (clock_data->type == TI_DEV_CLK_TABLE_TYPE_MUX)) {
		const struct ti_dev_clk_data *parent_clk_data;
		/* Send to parent */
		clk_idx_val = ti_device_clk_get_parent(dev, clk_idx_val);
		parent = ti_dev_get_clk(dev, clk_idx_val);
		parent_clk_data = ti_get_dev_clk_data(dev, clk_idx_val);
		if (parent_clk_data != NULL) {
			/* We are sending to parent, so use that div instead */
			div_var = parent_clk_data->div;
		}
		if (parent == NULL) {
			/* Mux parent clock not present */
			return 0U;
		}
	}

	if (clock_data->type == TI_DEV_CLK_TABLE_TYPE_OUTPUT) {
		/* div is only for input clocks */
		div_var = 1U;
	}

	if (clock_data->modify_parent_freq == 0U) {
		if (div_var != 0U) {
			ret_freq = ti_clk_get_freq(parent) / div_var;
			if ((ret_freq < min_freq_hz) || (ret_freq > max_freq_hz)) {
				ret_freq = 0U;
			}
		}
		return ret_freq;
	}

	/* Try to modify the frequency */
	changed = false;
	if (clock_data->type == TI_DEV_CLK_TABLE_TYPE_OUTPUT) {
		/*
		 * This is the only place device output clocks can have their
		 * frequency changed, from their own device.
		 */
		ret_freq = ti_clk_value_set_freq(parent, target_freq_hz,
						 min_freq_hz, max_freq_hz,
						 &changed);
	} else {
		ret_freq = ti_clk_generic_set_freq_parent(NULL, parent,
							  target_freq_hz,
							  min_freq_hz,
							  max_freq_hz,
							  &changed,
							  div_var);
	}

	return ret_freq;
}

bool ti_device_clk_set_freq(struct ti_device *dev, ti_dev_clk_idx_t clk_idx,
			    uint32_t min_freq_hz, uint32_t target_freq_hz,
			    uint32_t max_freq_hz)
{
	return ti_dev_clk_set_freq(dev, clk_idx, min_freq_hz, target_freq_hz,
				   max_freq_hz) != 0U;
}

uint32_t ti_device_clk_get_freq(struct ti_device *dev, ti_dev_clk_idx_t clk_idx)
{
	struct ti_clk *clkp;
	const struct ti_dev_clk_data *clock_data;
	uint32_t freq_hz;

	clkp = ti_dev_get_clk(dev, clk_idx);
	clock_data = ti_get_dev_clk_data(dev, clk_idx);
	if ((clkp == NULL) || (clock_data == NULL)) {
		freq_hz = 0U;
	} else {
		freq_hz = ti_clk_get_freq(clkp);
		if (clock_data->type != TI_DEV_CLK_TABLE_TYPE_OUTPUT) {
			if (clock_data->div != 0U) {
				freq_hz /= clock_data->div;
			} else {
				freq_hz = 0U;
			}
		}
	}

	return freq_hz;
}

void ti_device_clk_enable(struct ti_device *dev, ti_dev_clk_idx_t clk_idx)
{
	struct ti_dev_clk *dev_clkp;
	struct ti_clk *clkp = NULL;

	dev_clkp = ti_get_dev_clk(dev, clk_idx);
	if ((dev_clkp != NULL) && (0U == (dev_clkp->flags & TI_DEV_CLK_FLAG_DISABLE))) {
		clkp = ti_dev_get_clk(dev, clk_idx);
	}

	if (clkp != NULL) {
		(void)ti_clk_get(clkp);
	}
}

void ti_device_clk_disable(struct ti_device *dev, ti_dev_clk_idx_t clk_idx)
{
	struct ti_dev_clk *dev_clkp;
	struct ti_clk *clkp = NULL;

	dev_clkp = ti_get_dev_clk(dev, clk_idx);
	if ((dev_clkp != NULL) && (0U == (dev_clkp->flags & TI_DEV_CLK_FLAG_DISABLE))) {
		clkp = ti_dev_get_clk(dev, clk_idx);
	}

	if (clkp != NULL) {
		ti_clk_put(clkp);
	}
}

void ti_device_clk_init(struct ti_device *dev, ti_dev_clk_idx_t clk_idx)
{
	struct ti_clk *clkp = NULL;
	struct ti_dev_clk *dev_clkp;
	const struct ti_dev_clk_data *dev_clk_datap;

	dev_clkp = ti_get_dev_clk(dev, clk_idx);
	dev_clk_datap = ti_get_dev_clk_data(dev, clk_idx);

	/* Don't configure parent clocks for anything until host does */
	if ((dev_clk_datap != NULL) && (dev_clkp != NULL)) {
		if (dev_clk_datap->type == TI_DEV_CLK_TABLE_TYPE_PARENT) {
			dev_clkp->flags |= TI_DEV_CLK_FLAG_DISABLE;
		}
	}

	if ((dev_clk_datap != NULL) && (dev_clkp != NULL)) {
		clkp = ti_clk_lookup((ti_clk_idx_t) dev_clk_datap->clk);
	}
	if (clkp != NULL) {
		/* It's in another devgroup, don't attempt to bring it up */
		if ((clkp->flags & TI_CLK_FLAG_INITIALIZED) == 0U) {
			dev_clkp->flags |= TI_DEV_CLK_FLAG_DISABLE;
		}
	}
}
