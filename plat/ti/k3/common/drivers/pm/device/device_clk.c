/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <limits.h>
#include <types/errno.h>
#include <device.h>
#include <device_clk.h>
#include <clk.h>
#include <clk_mux.h>
#include <lib/trace.h>

struct dev_clk *get_dev_clk(struct device *dev, dev_clk_idx_t idx)
{
	const struct dev_data *data = get_dev_data(dev);
	const struct devgroup *devgrp = dev_data_lookup_devgroup(data);
	struct dev_clk *ret = NULL;

	if ((idx < data->n_clocks) && (devgrp != NULL)) {
		uint32_t offset = data->dev_clk_idx;

		if (clk_id_valid(devgrp->dev_clk_data[offset + idx].clk)) {
			ret = devgrp->dev_clk + offset + idx;
		}
	}

	return ret;
}

const struct dev_clk_data *get_dev_clk_data(struct device *dev, dev_clk_idx_t idx)
{
	const struct dev_data *data = get_dev_data(dev);
	const struct devgroup *devgrp = dev_data_lookup_devgroup(data);
	const struct dev_clk_data *ret = NULL;

	if ((idx < data->n_clocks) && (devgrp != NULL)) {
		uint32_t offset = data->dev_clk_idx;

		ret = devgrp->dev_clk_data + offset + idx;
	}

	return ret;
}

struct clk *dev_get_clk(struct device *dev, dev_clk_idx_t idx)
{
	const struct dev_clk_data *entry = get_dev_clk_data(dev, idx);

	return entry ? clk_lookup((clk_idx_t) entry->clk) : NULL;
}

bool device_clk_set_gated(struct device *dev, dev_clk_idx_t clk_idx, bool gated)
{
	const struct dev_data *data = get_dev_data(dev);
	const struct devgroup *devgrp = dev_data_lookup_devgroup(data);
	struct dev_clk *dev_clkp = get_dev_clk(dev, clk_idx);
	struct clk *clkp = NULL;
	bool is_enabled = false;
	bool ret = true;

	if ((dev_clkp == NULL) || (devgrp == NULL)) {
		ret = false;
	} else {
		bool is_gated;

		is_gated = (((uint32_t) (dev_clkp->flags) & DEV_CLK_FLAG_DISABLE) > 0U ?
			    true : false);
		if (is_gated != gated) {
			is_enabled = (dev->flags & DEV_FLAG_ENABLED_MASK) != 0UL;
			clkp = clk_lookup(
				(clk_idx_t) devgrp->dev_clk_data[data->dev_clk_idx + clk_idx].clk);
			if (!clkp) {
				/* Clock lookup failed */
				ret = false;
			}
		}
	}

	if ((clkp != NULL) && ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U)) {
		/* Clock not yet initialized (outside devgroup) */
		ret = false;
	} else if (clkp && gated) {
		dev_clkp->flags |= DEV_CLK_FLAG_DISABLE;
		if (is_enabled) {
			clk_put(clkp);

			if (0U == (dev_clkp->flags & DEV_CLK_FLAG_ALLOW_SSC)) {
				clk_ssc_allow(clkp);
			}

			if (0U == (dev_clkp->flags &
				   DEV_CLK_FLAG_ALLOW_FREQ_CHANGE)) {
				clk_freq_change_allow(clkp);
			}
		}
	} else if (clkp != NULL) {
		dev_clkp->flags &= (uint8_t) ~DEV_CLK_FLAG_DISABLE;
		if (is_enabled) {
			if (clk_get(clkp)) {
				if (0U == (dev_clkp->flags & DEV_CLK_FLAG_ALLOW_SSC)) {
					clk_ssc_block(clkp);
				}

				if (0U == (dev_clkp->flags &
					   DEV_CLK_FLAG_ALLOW_FREQ_CHANGE)) {
					clk_freq_change_block(clkp);
				}
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

bool device_clk_get_hw_ready(struct device *dev, dev_clk_idx_t clk_idx)
{
	const struct dev_data *data = get_dev_data(dev);
	const struct devgroup *devgroup_ptr = dev_data_lookup_devgroup(data);
	struct clk *clkp;
	bool ret = true;


	if (devgroup_ptr == NULL) {
		ret = false;
	}

	if (ret) {
		clkp = dev_get_clk(dev, clk_idx);
		if (clkp == NULL) {
			ret = false;
		}
	}

	if (ret) {
		switch (devgroup_ptr->dev_clk_data[data->dev_clk_idx + clk_idx].type) {
		case DEV_CLK_TABLE_TYPE_INPUT:
			ret = clk_get_state(clkp) == CLK_HW_STATE_ENABLED;
			break;
		case DEV_CLK_TABLE_TYPE_PARENT:
			ret = clk_get_state(clkp) == CLK_HW_STATE_ENABLED;
			break;
		case DEV_CLK_TABLE_TYPE_MUX:
			ret = clk_get_state(clkp) == CLK_HW_STATE_ENABLED;
			break;
		case DEV_CLK_TABLE_TYPE_OUTPUT:
			ret = true;
			break;
		default:
			ret = false;
			break;
		}
	}

	return ret;
}

bool device_clk_get_sw_gated(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct dev_clk *dev_clkp = get_dev_clk(dev, clk_idx);

	return dev_clkp && (dev_clkp->flags & DEV_CLK_FLAG_DISABLE);
}

void device_clk_set_ssc(struct device *dev, dev_clk_idx_t clk_idx, bool allow)
{
	struct dev_clk *dev_clkp = get_dev_clk(dev, clk_idx);
	struct clk *clkp;
	bool is_allowed;

	if (!dev_clkp) {
		/* Nothing to do */
	} else {
		is_allowed = (bool) !!(dev_clkp->flags & DEV_CLK_FLAG_ALLOW_SSC);
		if (is_allowed == allow) {
			/* Nothing to do */
		} else {
			dev_clkp->flags ^= DEV_CLK_FLAG_ALLOW_SSC;

			if ((dev->flags & DEV_FLAG_ENABLED_MASK) == 0UL) {
				/* Nothing to do */
			} else {
				if ((dev_clkp->flags & DEV_CLK_FLAG_DISABLE) != 0U) {
					/* Nothing to do */
				} else {
					clkp = dev_get_clk(dev, clk_idx);

					if (!clkp) {
						/* fail */
					} else if (allow) {
						clk_ssc_allow(clkp);
					} else {
						clk_ssc_block(clkp);
					}
				}
			}
		}
	}
}

bool device_clk_get_ssc(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct dev_clk *dev_clkp = get_dev_clk(dev, clk_idx);

	return dev_clkp && (dev_clkp->flags & DEV_CLK_FLAG_ALLOW_SSC);
}

bool device_clk_get_hw_ssc(struct device *dev __attribute__(
				   (unused)), dev_clk_idx_t clk_idx __maybe_unused)
{
	return false;
}

void device_clk_set_freq_change(struct device *dev, dev_clk_idx_t clk_idx, bool allow)
{
	struct dev_clk *dev_clkp = get_dev_clk(dev, clk_idx);
	struct clk *clkp;
	bool is_allowed;

	if (!dev_clkp) {
		/* Nothing to do */
	} else {
		is_allowed = (bool) !!(dev_clkp->flags & DEV_CLK_FLAG_ALLOW_FREQ_CHANGE);
		if (is_allowed == allow) {
			/* Nothing to do */
		} else {
			dev_clkp->flags ^= DEV_CLK_FLAG_ALLOW_FREQ_CHANGE;

			if ((dev->flags & DEV_FLAG_ENABLED_MASK) == 0UL) {
				/* Nothing to do*/
			} else {
				if ((dev_clkp->flags & DEV_CLK_FLAG_DISABLE) != 0U) {
					/* Nothing to do */
				} else {
					clkp = dev_get_clk(dev, clk_idx);

					if (!clkp) {
						/* fail */
					} else if (allow) {
						clk_freq_change_allow(clkp);
					} else {
						clk_freq_change_block(clkp);
					}
				}
			}
		}
	}
}

bool device_clk_get_freq_change(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct dev_clk *dev_clkp = get_dev_clk(dev, clk_idx);

	return dev_clkp && (dev_clkp->flags & DEV_CLK_FLAG_ALLOW_FREQ_CHANGE);
}

void device_clk_set_input_term(struct device *dev, dev_clk_idx_t clk_idx, bool enable)
{
	struct dev_clk *dev_clkp = get_dev_clk(dev, clk_idx);
	bool is_term;

	if (!dev_clkp) {
		/* Do nothing  - return */
	} else {
		is_term = (bool) !!(dev_clkp->flags & DEV_CLK_FLAG_INPUT_TERM);
		if (is_term == enable) {
			/* Do nothing  - return */
		} else {
			dev_clkp->flags ^= DEV_CLK_FLAG_INPUT_TERM;
		}
	}

}

bool device_clk_get_input_term(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct dev_clk *dev_clkp = get_dev_clk(dev, clk_idx);

	return dev_clkp && (dev_clkp->flags & DEV_CLK_FLAG_INPUT_TERM);
}

bool device_clk_set_parent(struct device *dev, dev_clk_idx_t clk_idx, dev_clk_idx_t parent_idx)
{
	struct clk *clkp;
	const struct dev_clk_data *clock_data;
	const struct dev_clk_data *parent_data = NULL;
	bool ret = true;

	clkp = dev_get_clk(dev, clk_idx);
	clock_data = get_dev_clk_data(dev, clk_idx);
	if ((clkp == NULL) || (clock_data == NULL)) {
		ret = false;
	}


	if (ret && (clock_data->type != DEV_CLK_TABLE_TYPE_MUX)) {
		ret = false;
	}

	if (ret) {
		parent_data = get_dev_clk_data(dev, parent_idx);
		if (parent_data == NULL) {
			ret = false;
		}
	}

	if (ret && (parent_data->type != DEV_CLK_TABLE_TYPE_PARENT)) {
		ret = false;
	}

	/* Make sure it's within this clock muxes parents */
	if (ret && ((parent_idx - clk_idx) > clock_data->idx)) {
		ret = false;
	}

	while (ret && clkp && ((clk_get_data(clkp)->type != CLK_TYPE_MUX))) {
		const struct clk_parent *p;

		p = clk_get_parent(clkp);
		if (p != NULL) {
			clkp = clk_lookup((clk_idx_t) p->clk);
		} else {
			clkp = NULL;
		}
	}

	if (!clkp) {
		ret = false;
	}

	if (ret) {
		/* FIXME: Check for other uses of this mux */
		ret = clk_set_parent(clkp, parent_data->idx);
	}

	return ret;
}

dev_clk_idx_t device_clk_get_parent(struct device *dev, dev_clk_idx_t clk_idx)
{
	const struct dev_data *data = get_dev_data(dev);
	const struct devgroup *devgroup_ptr = dev_data_lookup_devgroup(data);
	const struct clk_parent *p = NULL;
	struct clk *clkp;
	const struct dev_clk_data *clock_data;
	bool fail = false;
	dev_clk_idx_t ret = DEV_CLK_ID_NONE;
	dev_clk_idx_t i;

	clkp = dev_get_clk(dev, clk_idx);
	clock_data = get_dev_clk_data(dev, clk_idx);
	if ((clkp == NULL) || (clock_data == NULL) || (devgroup_ptr == NULL)) {
		fail = true;
	}

	if (!fail && (clock_data->type != DEV_CLK_TABLE_TYPE_MUX)) {
		fail = true;
	}

	while (!fail && (clkp != NULL) && (clk_get_data(clkp)->type != CLK_TYPE_MUX)) {
		p = clk_get_parent(clkp);
		if (p != NULL) {
			clkp = clk_lookup((clk_idx_t) p->clk);
		} else {
			clkp = NULL;
		}
	}

	if (!fail) {
		p = clk_get_parent(clkp);
		if (!p) {
			fail = true;
		}
	}

	if (!fail) {
		uint32_t offset = data->dev_clk_idx;

		fail = true;
		for (i = 0U; (i < clock_data->idx) && fail; i++) {
			if (devgroup_ptr->dev_clk_data[offset + i + clk_idx + 1U].clk == p->clk) {
				ret = i + clk_idx + 1U;
				fail = false;
			}
		}
	}

	return ret;
}

dev_clk_idx_t device_clk_get_num_parents(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct clk *clkp;
	const struct dev_clk_data *clock_data;
	dev_clk_idx_t ret;

	clkp = dev_get_clk(dev, clk_idx);
	clock_data = get_dev_clk_data(dev, clk_idx);
	if ((clkp == NULL) || (clock_data == NULL)) {
		ret = DEV_CLK_ID_NONE;
	} else if (clock_data->type != DEV_CLK_TABLE_TYPE_MUX) {
		if (clk_get_parent(clkp) != NULL) {
			ret = 1U;
		} else {
			ret = 0U;
		}
	} else {
		ret = (dev_clk_idx_t) clock_data->idx;
	}

	return ret;
}

/**
 * \brief Set or query the frequency of a device's clock
 *
 * This locates the correct clock and calls the internal clock API
 * query/set function. If the clock is a mux type, we instead send the
 * request to the parent. This is because calling set freq on the mux may
 * switch the mux which is not what we want on muxes that are exposed on
 * devices.
 *
 * \param dev
 * The device ID that the clock is connected to.
 *
 * \param clk_idx
 * The index of the clock on this device.
 *
 * \param min_freq_hz
 * The minimum acceptable frequency (Hz).
 *
 * \param target_freq_hz
 * The clock API will attempt to return a frequency as close as possible to the
 * target frequency (Hz).
 *
 * \param max_freq_hz
 * The maximum acceptable frequency (Hz).
 *
 * \param query
 * True if this is a query (make no changes), false if this is a set (make
 * changes if possible).
 *
 * \return
 * The actual frequency possible (query=true) or set (query=false). Returns
 * 0 if a frequency could not be found within the limits.
 */
static uint32_t dev_clk_set_freq(struct device *dev, dev_clk_idx_t clk_idx,
				 uint32_t min_freq_hz, uint32_t target_freq_hz,
				 uint32_t max_freq_hz, bool query)
{
	const struct dev_data *data = get_dev_data(dev);
	const struct devgroup *devgroup_ptr = dev_data_lookup_devgroup(data);
	const struct dev_clk_data *clock_data;
	struct clk *parent = NULL;
	struct clk *unblock1 = NULL;
	struct clk *unblock2 = NULL;
	uint32_t div_var = 1U;
	bool done = false;
	uint32_t ret_freq = 0U;
	dev_clk_idx_t clk_idx_val = clk_idx;

	if (devgroup_ptr == NULL) {
		done = true;
	}

	if (!done) {
		clock_data = get_dev_clk_data(dev, clk_idx_val);
		if (clock_data == NULL) {
			/* Invalid clock idx */
			done = true;
		}
	}

	if (!done) {
		parent = dev_get_clk(dev, clk_idx_val);
		if (parent == NULL) {
			/* Parent not present */
			done = true;
		}
	}

	if (!done) {
		/* Assign div based on selected clock */
		div_var = clock_data->div;

		/*
		 * We drop the block count on up to two clocks. The clock
		 * id we are directly controlling is the first one.
		 */
		if (!device_clk_get_freq_change(dev, clk_idx_val)) {
			if (((dev->flags & DEV_FLAG_ENABLED_MASK) != 0UL) &&
			    !device_clk_get_sw_gated(dev, clk_idx_val)) {
				unblock1 = parent;
			}
		}

		if ((clk_get_data(parent)->type == CLK_TYPE_MUX) &&
		    (clock_data->type == DEV_CLK_TABLE_TYPE_MUX)) {
			const struct dev_clk_data *parent_clk_data;
			/* Send to parent */
			clk_idx_val = device_clk_get_parent(dev, clk_idx_val);
			parent = dev_get_clk(dev, clk_idx_val);
			parent_clk_data = get_dev_clk_data(dev, clk_idx_val);
			if (parent_clk_data != NULL) {
				/* We are sending to parent, so use that div instead */
				div_var = parent_clk_data->div;
			}
			if (parent == NULL) {
				/* Mux parent clock not present */
				done = true;
			} else if (!device_clk_get_freq_change(dev, clk_idx_val)) {
				/*
				 * If we are changing a clock with a device
				 * clock mux parent, unblock that clock as
				 * well. (Note, device clock mux parents are
				 * unblocked by default but can be blocked via
				 * the TI-SCI API).
				 */
				if (((dev->flags & DEV_FLAG_ENABLED_MASK) != 0UL) &&
				    !device_clk_get_sw_gated(dev, clk_idx_val)) {
					unblock2 = parent;
				}
			} else {
				/* Do Nothing */
			}
		} else if (clock_data->type == DEV_CLK_TABLE_TYPE_PARENT) {
			/*
			 * If we are changing a mux parent, also unblock the
			 * child clock it connects to. First we need to walk
			 * back the table to find the child clock.
			 */
			int32_t i;
			uint32_t offset = data->dev_clk_idx;

			for (i = (int32_t) clk_idx_val - 1; i >= 0L; i--) {
				if (devgroup_ptr->dev_clk_data[(int32_t) offset + i].type ==
				    DEV_CLK_TABLE_TYPE_MUX) {
					break;
				}
			}
			/*
			 * Make sure the mux has currently selected the target
			 * clock.
			 */
			if ((i >= 0) && (clk_idx_val == device_clk_get_parent(dev, (uint16_t) i))) {
				if (device_clk_get_freq_change(dev, (uint16_t) i)) {
					if (((dev->flags & DEV_FLAG_ENABLED_MASK) != 0UL) &&
					    !device_clk_get_sw_gated(dev, clk_idx_val)) {
						unblock2 = dev_get_clk(dev, (uint16_t) i);
					}
				}
			}
		} else {
			/* Do Nothing */
		}
	}

	if (!done) {
		if (clock_data->type == DEV_CLK_TABLE_TYPE_OUTPUT) {
			/* div is only for input clocks */
			div_var = 1U;
		}

		if (clock_data->modify_parent_freq == 0U) {
			ret_freq = clk_get_freq(parent) / div_var;
			if ((ret_freq < min_freq_hz) || (ret_freq > max_freq_hz)) {
				ret_freq = 0U;
			}
			done = true;
		}
	}

	if (!done) {
		bool changed;

		changed = false;
		/* Drop unblock refs as necessary */
		if (unblock1 != NULL) {
			clk_freq_change_allow(unblock1);
		}
		if (unblock2 != NULL) {
			clk_freq_change_allow(unblock2);
		}
		/* Try to modify the frequency */

		if (clock_data->type == DEV_CLK_TABLE_TYPE_OUTPUT) {
			/*
			 * This is the only place device output clocks can have their
			 * frequency changed, from their own device.
			 */
			ret_freq = clk_value_set_freq(parent, target_freq_hz, min_freq_hz,
						      max_freq_hz, query, &changed);
		} else {
			ret_freq = clk_generic_set_freq_parent(NULL, parent,
							       target_freq_hz,
							       min_freq_hz,
							       max_freq_hz,
							       query, &changed,
							       div_var);
		}

		/* Put the block refs back */
		if (unblock1 != NULL) {
			clk_freq_change_block(unblock1);
		}
		if (unblock2 != NULL) {
			clk_freq_change_block(unblock2);
		}
	}

	return ret_freq;
}

bool device_clk_set_freq(struct device *dev, dev_clk_idx_t clk_idx, uint32_t min_freq_hz,
			 uint32_t target_freq_hz, uint32_t max_freq_hz)
{
	return dev_clk_set_freq(dev, clk_idx, min_freq_hz,
				target_freq_hz, max_freq_hz, false) != 0UL;
}

uint32_t device_clk_query_freq(struct device *dev, dev_clk_idx_t clk_idx,
			       uint32_t min_freq_hz, uint32_t target_freq_hz,
			       uint32_t max_freq_hz)
{
	return dev_clk_set_freq(dev, clk_idx, min_freq_hz,
				target_freq_hz, max_freq_hz, true);
}

uint32_t device_clk_get_freq(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct clk *clkp;
	const struct dev_clk_data *clock_data;
	uint32_t freq_hz;

	clkp = dev_get_clk(dev, clk_idx);
	clock_data = get_dev_clk_data(dev, clk_idx);
	if ((clkp == NULL) || (clock_data == NULL)) {
		freq_hz = 0U;
	} else {
		freq_hz = clk_get_freq(clkp);
		if (clock_data->type != DEV_CLK_TABLE_TYPE_OUTPUT) {
			freq_hz /= clock_data->div;
		}
	}

	return freq_hz;
}

void device_clk_enable(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct dev_clk *dev_clkp;
	struct clk *clkp = NULL;

	dev_clkp = get_dev_clk(dev, clk_idx);
	if ((dev_clkp != NULL) && (0U == (dev_clkp->flags & DEV_CLK_FLAG_DISABLE))) {
		clkp = dev_get_clk(dev, clk_idx);
	}

	if (clkp != NULL) {
		if (clk_get(clkp)) {
			if (0U == (dev_clkp->flags & DEV_CLK_FLAG_ALLOW_SSC)) {
				clk_ssc_block(clkp);
			}

			if (0U == (dev_clkp->flags & DEV_CLK_FLAG_ALLOW_FREQ_CHANGE)) {
				clk_freq_change_block(clkp);
			}
		}
	}
}

void device_clk_disable(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct dev_clk *dev_clkp;
	struct clk *clkp = NULL;

	dev_clkp = get_dev_clk(dev, clk_idx);
	if ((dev_clkp != NULL) && (0U == (dev_clkp->flags & DEV_CLK_FLAG_DISABLE))) {
		clkp = dev_get_clk(dev, clk_idx);
	}

	if (clkp != NULL) {
		clk_put(clkp);

		if (0U == (dev_clkp->flags & DEV_CLK_FLAG_ALLOW_SSC)) {
			clk_ssc_allow(clkp);
		}

		if (0U == (dev_clkp->flags & DEV_CLK_FLAG_ALLOW_FREQ_CHANGE)) {
			clk_freq_change_allow(clkp);
		}
	}
}

void device_clk_init(struct device *dev, dev_clk_idx_t clk_idx)
{
	struct clk *clkp = NULL;
	struct dev_clk *dev_clkp;
	const struct dev_clk_data *dev_clk_datap;

	dev_clkp = get_dev_clk(dev, clk_idx);
	dev_clk_datap = get_dev_clk_data(dev, clk_idx);

	/* Don't configure parent clocks for anything until host does */
	if ((dev_clk_datap != NULL) && (dev_clkp != NULL)) {
		if (dev_clk_datap->type == DEV_CLK_TABLE_TYPE_PARENT) {
			dev_clkp->flags |= DEV_CLK_FLAG_DISABLE |
				DEV_CLK_FLAG_ALLOW_SSC |
				DEV_CLK_FLAG_ALLOW_FREQ_CHANGE;
		}
	}

	if ((dev_clk_datap != NULL) && (dev_clkp != NULL)) {
		clkp = clk_lookup((clk_idx_t) dev_clk_datap->clk);
	}
	if (clkp != NULL) {
		const struct clk_data *clock_data = clk_get_data(clkp);

		/* It's in another devgroup, don't attempt to bring it up */
		if ((clkp->flags & CLK_FLAG_INITIALIZED) == 0U) {
			dev_clkp->flags |= DEV_CLK_FLAG_DISABLE;
		}

		/*
		 * For clocks with this flag set, default all dev_clk's to
		 * allow. Otherwise default all dev_clk's to block.
		 */
		if ((clock_data != NULL) && ((clock_data->flags &
					      CLK_DATA_FLAG_ALLOW_FREQ_CHANGE) != 0UL)) {
			dev_clkp->flags |= DEV_CLK_FLAG_ALLOW_FREQ_CHANGE;
		}
	}
}
