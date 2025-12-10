/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Clock Handler - SCMI Clock Protocol Implementation
 *
 * This module implements the SCMI (System Control and Management Interface)
 * clock protocol handlers for TI SoCs. It provides the interface between
 * SCMI clock requests from the host and the underlying TI clock framework,
 * handling clock enable/disable, frequency get/set, and parent selection
 * operations while enforcing security and exclusivity policies.
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#include <clk.h>
#include <common/debug.h>

#include <ti_clk.h>
#include <ti_clk_div.h>
#include <ti_clk_handler.h>
#include <ti_clk_mux.h>
#include <ti_device.h>
#include <ti_device_clk.h>
#include <ti_device_prepare.h>
#include <ti_hosts.h>
#include <drivers/clk.h>
#include <scmi_clock.h>

extern ti_scmi_clock_t clock_table[];
extern const size_t clock_table_size;

static int32_t set_clock_state(uint32_t dev_id, uint32_t clk_id, bool enable)
{
	struct ti_device *dev = NULL;
	dev_clk_idx_t clkidx = (dev_clk_idx_t) clk_id;
	int32_t ret;
	uint8_t host_id = HOST_ID_TIFS;

	VERBOSE("SET_CLOCK: clk_id=%d dev_id=%d enable=%d\n",
		clkidx, dev_id, enable);

	ret = device_prepare_exclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return ret;
	}

	ti_device_clk_set_freq_change(dev, clkidx, true);

	if (!ti_device_clk_set_gated(dev, clkidx, false)) {
		return -EINVAL;
	}

	return 0;
}

static int32_t enable_clock_handler(uint32_t dev_id, uint32_t clk_id)
{
	return set_clock_state(dev_id, clk_id, true);
}

static int32_t disable_clock_handler(uint32_t dev_id, uint32_t clk_id)
{
	return set_clock_state(dev_id, clk_id, false);
}

static int32_t get_clock_handler(uint32_t dev_id, uint32_t clk_id)
{
	struct ti_device *dev = NULL;
	dev_clk_idx_t clkidx = (dev_clk_idx_t) clk_id;
	uint8_t host_id = HOST_ID_TIFS;
	uint8_t prog;
	int32_t ret;

	VERBOSE("GET_CLOCK: clk_id=%d dev_id=%d\n", clkidx, dev_id);

	ret = device_prepare_nonexclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return 0;
	}

	prog = (uint8_t) (ti_device_clk_get_sw_gated(dev, clkidx) ?
			  CLOCK_SW_STATE_UNREQ :
			  CLOCK_SW_STATE_AUTO);

	return (int32_t)prog;
}

static int32_t set_clock_parent_handler(uint32_t dev_id, uint32_t clk_id, uint32_t parent_id)
{
	struct ti_device *dev = NULL;
	dev_clk_idx_t clkidx = (dev_clk_idx_t) clk_id;
	dev_clk_idx_t parent = (dev_clk_idx_t) parent_id;
	uint8_t host_id = HOST_ID_TIFS;
	int32_t ret;

	VERBOSE("SET_CLOCK_PARENT: clk_id=%d dev_id=%d parent=%d\n",
		clkidx, dev_id, parent);

	ret = device_prepare_exclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return ret;
	}

	if (!ti_device_clk_set_parent(dev, clkidx, parent)) {
		return -EINVAL;
	}

	return 0;
}

static int32_t get_clock_parent_handler(uint32_t dev_id, uint32_t clk_id, uint32_t *parent_id)
{
	struct ti_device *dev = NULL;
	dev_clk_idx_t clkidx = (dev_clk_idx_t) clk_id;
	uint8_t host_id = HOST_ID_TIFS;
	dev_clk_idx_t parent;
	int32_t ret;

	VERBOSE("GET_CLOCK_PARENT: clk_id=%d dev_id=%d\n",
		clkidx, dev_id);

	ret = device_prepare_nonexclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return ret;
	}

	parent = ti_device_clk_get_parent(dev, clkidx);

	if (parent == DEV_CLK_ID_NONE) {
		return -EINVAL;
	}

	*parent_id = (uint32_t) parent;
	return 0;
}

static int32_t get_num_clock_parents_handler(uint32_t dev_id, uint32_t clk_id)
{
	struct ti_device *dev = NULL;
	dev_clk_idx_t clkidx = (dev_clk_idx_t) clk_id;
	uint8_t host_id = HOST_ID_TIFS;
	dev_clk_idx_t num_parents;
	int32_t ret;

	VERBOSE("GET_NUM_CLOCK_PARENTS: clk_id=%d dev_id=%d\n",
		clkidx, dev_id);

	ret = device_prepare_nonexclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return 0;
	}

	num_parents = ti_device_clk_get_num_parents(dev, clkidx);

	if (num_parents == DEV_CLK_ID_NONE) {
		return 0;
	}

	return (int32_t) num_parents;
}

static int32_t set_freq_handler(uint32_t dev_id, uint32_t clk_id, uint64_t target_freq)
{
	struct ti_device *dev = NULL;
	dev_clk_idx_t clkidx = (dev_clk_idx_t) clk_id;
	uint64_t min_freq_hz = target_freq / 10U * 9U;
	uint64_t max_freq_hz = target_freq / 10U * 11U;
	uint8_t host_id = HOST_ID_TIFS;
	int32_t ret;

	VERBOSE("SET_FREQ: clk_id=%d dev_id=%d\n", clkidx, dev_id);

	ret = device_prepare_exclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return ret;
	}

	if ((min_freq_hz > target_freq) || (target_freq > max_freq_hz)) {
		return -EINVAL;
	}

	if (!ti_device_clk_set_freq(dev, clkidx, (uint32_t) min_freq_hz,
				 (uint32_t) target_freq,
				 (uint32_t) max_freq_hz)) {
		return -EINVAL;
	}

	return 0;
}

static uint64_t get_freq_handler(uint32_t dev_id, uint32_t clk_id)
{
	struct ti_device *dev = NULL;
	dev_clk_idx_t clkidx = (dev_clk_idx_t) clk_id;
	uint8_t host_id = HOST_ID_TIFS;
	int32_t ret;

	VERBOSE("GET_FREQ: clk_id=%d dev_id=%d\n", clkidx, dev_id);

	ret = device_prepare_nonexclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return 0U;
	}

	return ti_device_clk_get_freq(dev, clkidx);
}

static ti_scmi_clock_t *scmi_id_to_clock(unsigned long scmi_id)
{
	if (scmi_id >= clock_table_size) {
		return NULL;
	}

	return &clock_table[scmi_id];
}

static int ti_clk_enable(unsigned long scmi_id)
{
	ti_scmi_clock_t *clock;

	clock = scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return -EINVAL;
	}

	return enable_clock_handler(clock->dev_id, clock->clock_id);
}

static void ti_clk_disable(unsigned long scmi_id)
{
	ti_scmi_clock_t *clock;

	clock = scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return;
	}

	disable_clock_handler(clock->dev_id, clock->clock_id);
}

static bool ti_clk_is_enabled(unsigned long scmi_id)
{
	ti_scmi_clock_t *clock;

	clock = scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return false;
	}

	return !!get_clock_handler(clock->dev_id, clock->clock_id);
}

static unsigned long ti_clk_get_rate(unsigned long scmi_id)
{
	ti_scmi_clock_t *clock;

	clock = scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return 0;
	}

	return (unsigned long)get_freq_handler(clock->dev_id, clock->clock_id);
}

static int ti_clk_set_rate(unsigned long scmi_id, unsigned long rate,
			    unsigned long *orate)
{
	ti_scmi_clock_t *clock;
	int32_t ret;

	clock = scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return -EINVAL;
	}

	ret = set_freq_handler(clock->dev_id, clock->clock_id, (uint64_t)rate);
	if (ret == 0 && orate != NULL) {
		*orate = (unsigned long)get_freq_handler(clock->dev_id, clock->clock_id);
	}

	return ret;
}

static int ti_clk_get_possible_parents_num(unsigned long scmi_id)
{
	ti_scmi_clock_t *clock;

	clock = scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return 0;
	}
	return get_num_clock_parents_handler(clock->dev_id, clock->clock_id);
}

static int ti_clk_get_parent(unsigned long scmi_id)
{
	ti_scmi_clock_t *clock = scmi_id_to_clock(scmi_id);
	uint32_t parent_id;
	int32_t status;
	int32_t n_parents;
	ti_scmi_clock_t *parent;

	if (clock == NULL) {
		return -EINVAL;
	}

	status = get_clock_parent_handler(clock->dev_id, clock->clock_id, &parent_id);
	if (status != 0) {
		return -EINVAL;
	}

	n_parents = get_num_clock_parents_handler(clock->dev_id, clock->clock_id);
	if (n_parents > 1) {
		/* Loop through the parents to identify the parent that matches the returned ID */
		for (unsigned int i = 1; i <= (unsigned int)n_parents; i++) {
			if (scmi_id >= i) {
				parent = scmi_id_to_clock(scmi_id - i);
				if (parent != NULL && parent->clock_id == parent_id) {
					return scmi_id - i;
				}
			}
		}
	}

	return -EINVAL;
}

static int ti_clk_set_parent(unsigned long scmi_id, unsigned long parent_scmi_id)
{
	ti_scmi_clock_t *clock = scmi_id_to_clock(scmi_id);
	ti_scmi_clock_t *parent = scmi_id_to_clock(parent_scmi_id);

	if (clock == NULL || parent == NULL) {
		return -EINVAL;
	}

	return set_clock_parent_handler(clock->dev_id, clock->clock_id, parent->clock_id);
}

static const struct clk_ops ti_clk_ops = {
	.enable = ti_clk_enable,
	.disable = ti_clk_disable,
	.is_enabled = ti_clk_is_enabled,
	.get_rate = ti_clk_get_rate,
	.set_rate = ti_clk_set_rate,
	.get_possible_parents_num = ti_clk_get_possible_parents_num,
	.get_parent = ti_clk_get_parent,
	.set_parent = ti_clk_set_parent,
};

void ti_clk_handler_init(void)
{
	clk_register(&ti_clk_ops);
}
