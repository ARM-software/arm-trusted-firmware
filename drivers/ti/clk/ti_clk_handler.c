/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com
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

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

#include <common/debug.h>
#include <drivers/clk.h>

#include <ti_clk.h>
#include <ti_clk_div.h>
#include <ti_clk_handler.h>
#include <ti_clk_mux.h>
#include <ti_device.h>
#include <ti_device_clk.h>
#include <ti_device_prepare.h>
#include <ti_hosts.h>
#include <scmi_clock.h>


extern struct ti_scmi_clock clock_table[];
extern const size_t clock_table_size;

static int32_t ti_set_clock_state(uint32_t dev_id, uint32_t clk_id, bool enable)
{
	struct ti_device *dev = NULL;
	ti_dev_clk_idx_t clkidx = (ti_dev_clk_idx_t)clk_id;
	int32_t ret;
	uint8_t host_id = TI_HOST_ID_TIFS;

	VERBOSE("SET_CLOCK: clk_id=%u dev_id=%u enable=%u\n",
		clkidx, dev_id, enable);

	ret = ti_device_prepare_exclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return ret;
	}

	(void)enable;
	if (!ti_device_clk_set_gated(dev, clkidx, false)) {
		return -EINVAL;
	}

	return 0;
}

static bool ti_get_clock_handler(uint32_t dev_id, uint32_t clk_id)
{
	struct ti_device *dev = NULL;
	ti_dev_clk_idx_t clkidx = (ti_dev_clk_idx_t)clk_id;
	uint8_t host_id = TI_HOST_ID_TIFS;
	int32_t ret;

	VERBOSE("GET_CLOCK: clk_id=%u dev_id=%u\n", clkidx, dev_id);

	ret = ti_device_prepare_nonexclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return false;
	}

	return !ti_device_clk_get_sw_gated(dev, clkidx);
}

static int32_t ti_set_clock_parent_handler(uint32_t dev_id, uint32_t clk_id, uint32_t parent_id)
{
	struct ti_device *dev = NULL;
	ti_dev_clk_idx_t clkidx = (ti_dev_clk_idx_t)clk_id;
	ti_dev_clk_idx_t parent = (ti_dev_clk_idx_t)parent_id;
	uint8_t host_id = TI_HOST_ID_TIFS;
	int32_t ret;

	VERBOSE("SET_CLOCK_PARENT: clk_id=%u dev_id=%u parent=%u\n",
		clkidx, dev_id, parent);

	ret = ti_device_prepare_exclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return ret;
	}

	if (!ti_device_clk_set_parent(dev, clkidx, parent)) {
		return -EINVAL;
	}

	return 0;
}

static int32_t ti_get_clock_parent_handler(uint32_t dev_id, uint32_t clk_id, uint32_t *parent_id)
{
	struct ti_device *dev = NULL;
	ti_dev_clk_idx_t clkidx = (ti_dev_clk_idx_t)clk_id;
	uint8_t host_id = TI_HOST_ID_TIFS;
	ti_dev_clk_idx_t parent;
	int32_t ret;

	assert(parent_id != NULL);

	VERBOSE("GET_CLOCK_PARENT: clk_id=%u dev_id=%u\n",
		clkidx, dev_id);

	ret = ti_device_prepare_nonexclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return ret;
	}

	parent = ti_device_clk_get_parent(dev, clkidx);

	if (parent == TI_DEV_CLK_ID_NONE) {
		return -EINVAL;
	}

	*parent_id = (uint32_t)parent;
	return 0;
}

static int32_t ti_get_num_clock_parents_handler(uint32_t dev_id, uint32_t clk_id)
{
	struct ti_device *dev = NULL;
	ti_dev_clk_idx_t clkidx = (ti_dev_clk_idx_t)clk_id;
	uint8_t host_id = TI_HOST_ID_TIFS;
	ti_dev_clk_idx_t num_parents;
	int32_t ret;

	VERBOSE("GET_NUM_CLOCK_PARENTS: clk_id=%u dev_id=%u\n",
		clkidx, dev_id);

	ret = ti_device_prepare_nonexclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return 0;
	}

	num_parents = ti_device_clk_get_num_parents(dev, clkidx);

	if (num_parents == TI_DEV_CLK_ID_NONE) {
		return 0;
	}

	return (int32_t)num_parents;
}

static int32_t ti_set_freq_handler(uint32_t dev_id, uint32_t clk_id, uint64_t target_freq)
{
	struct ti_device *dev = NULL;
	ti_dev_clk_idx_t clkidx = (ti_dev_clk_idx_t)clk_id;
	uint64_t min_freq_hz;
	uint64_t max_freq_hz;
	uint8_t host_id = TI_HOST_ID_TIFS;
	int32_t ret;

	VERBOSE("SET_FREQ: clk_id=%u dev_id=%u\n", clkidx, dev_id);

	if (target_freq > (uint64_t)UINT32_MAX) {
		return -EINVAL;
	}

	if (target_freq < 10U) {
		min_freq_hz = 0U;
		max_freq_hz = target_freq * 2U;
	} else {
		min_freq_hz = target_freq / 10U * 9U;
		max_freq_hz = target_freq / 10U * 11U;
		if (max_freq_hz > (uint64_t)UINT32_MAX) {
			max_freq_hz = (uint64_t)UINT32_MAX;
		}
	}

	ret = ti_device_prepare_exclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return ret;
	}

	if (!ti_device_clk_set_freq(dev, clkidx, (uint32_t)min_freq_hz,
				    (uint32_t)target_freq,
				    (uint32_t)max_freq_hz)) {
		return -EINVAL;
	}

	return 0;
}

static uint64_t ti_get_freq_handler(uint32_t dev_id, uint32_t clk_id)
{
	struct ti_device *dev = NULL;
	ti_dev_clk_idx_t clkidx = (ti_dev_clk_idx_t)clk_id;
	uint8_t host_id = TI_HOST_ID_TIFS;
	int32_t ret;

	VERBOSE("GET_FREQ: clk_id=%u dev_id=%u\n", clkidx, dev_id);

	ret = ti_device_prepare_nonexclusive(host_id, dev_id, NULL, &dev);
	if (ret != 0) {
		return 0U;
	}

	return ti_device_clk_get_freq(dev, clkidx);
}

static struct ti_scmi_clock *ti_scmi_id_to_clock(unsigned long scmi_id)
{
	if (scmi_id >= clock_table_size) {
		return NULL;
	}

	return &clock_table[scmi_id];
}

static int ti_clk_enable(unsigned long scmi_id)
{
	struct ti_scmi_clock *clock;

	clock = ti_scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return -EINVAL;
	}

	return ti_set_clock_state(clock->dev_id, clock->clock_id, true);
}

static void ti_clk_disable(unsigned long scmi_id)
{
	struct ti_scmi_clock *clock;

	clock = ti_scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return;
	}

	ti_set_clock_state(clock->dev_id, clock->clock_id, false);
}

static bool ti_clk_is_enabled(unsigned long scmi_id)
{
	struct ti_scmi_clock *clock;

	clock = ti_scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return false;
	}

	return ti_get_clock_handler(clock->dev_id, clock->clock_id);
}

static unsigned long ti_clk_get_rate(unsigned long scmi_id)
{
	struct ti_scmi_clock *clock;

	clock = ti_scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return 0;
	}

	return (unsigned long)ti_get_freq_handler(clock->dev_id, clock->clock_id);
}

static int ti_clk_set_rate(unsigned long scmi_id, unsigned long rate,
			   unsigned long *orate)
{
	struct ti_scmi_clock *clock;
	int32_t ret;

	clock = ti_scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return -EINVAL;
	}

	if ((rate < clock->rates[0]) || (rate > clock->rates[1])) {
		return -EINVAL;
	}

	ret = ti_set_freq_handler(clock->dev_id, clock->clock_id, (uint64_t)rate);
	if (ret == 0 && orate != NULL) {
		*orate = (unsigned long)ti_get_freq_handler(clock->dev_id, clock->clock_id);
	}

	return ret;
}

static int ti_clk_get_possible_parents_num(unsigned long scmi_id)
{
	struct ti_scmi_clock *clock;

	clock = ti_scmi_id_to_clock(scmi_id);
	if (clock == NULL) {
		return 0;
	}
	return ti_get_num_clock_parents_handler(clock->dev_id, clock->clock_id);
}

static int ti_clk_get_parent(unsigned long scmi_id)
{
	struct ti_scmi_clock *clock = ti_scmi_id_to_clock(scmi_id);
	uint32_t parent_id;
	int32_t status;
	int32_t n_parents;
	struct ti_scmi_clock *parent;
	uint32_t total_parents;
	uint32_t n_reserved;
	uint32_t non_reserved;

	VERBOSE("GET_PARENT: scmi_id=%lu\n", scmi_id);

	if (scmi_id >= clock_table_size) {
		ERROR("GET_PARENT: Invalid scmi_id=%lu (table_size=%lu)\n",
		      scmi_id, clock_table_size);
		return -EINVAL;
	}

	if (clock == NULL) {
		ERROR("GET_PARENT: Clock lookup failed for scmi_id=%lu\n", scmi_id);
		return -EINVAL;
	}

	status = ti_get_clock_parent_handler(clock->dev_id, clock->clock_id, &parent_id);
	if (status != 0) {
		ERROR("GET_PARENT: get_clock_parent_handler failed for dev=%u clk=%u status=%d\n",
		      clock->dev_id, clock->clock_id, status);
		return -EINVAL;
	}

	n_parents = ti_get_num_clock_parents_handler(clock->dev_id, clock->clock_id);
	if (n_parents <= 0) {
		ERROR("GET_PARENT: Invalid n_parents=%d for dev=%u clk=%u\n",
		      n_parents, clock->dev_id, clock->clock_id);
		return -EINVAL;
	}

	if (n_parents == 1) {
		ERROR("GET_PARENT: Not a MUX clock (n_parents=1)\n");
		return -EINVAL;
	}

	/* Decode parent count: bits 0-7 = total, bits 8-15 = reserved */
	total_parents = n_parents & 0xFF;
	n_reserved = (n_parents >> 8) & 0xFF;

	if (total_parents == 0 || n_reserved > total_parents) {
		ERROR("GET_PARENT: Invalid parent counts total=%u reserved=%u\n",
		      total_parents, n_reserved);
		return -EINVAL;
	}

	non_reserved = total_parents - n_reserved;

	/* First, search through non-reserved parents (immediately before MUX) */
	for (unsigned int i = 1; i <= non_reserved && i <= scmi_id; i++) {
		parent = ti_scmi_id_to_clock(scmi_id - i);
		if (parent != NULL && parent->clock_id == parent_id) {
			VERBOSE("GET_PARENT: Found non-reserved parent at scmi_id=%lu\n",
				scmi_id - i);
			return scmi_id - i;
		}
	}

	/* If not found, search through reserved parents at end of clock table */
	if (n_reserved > 0 && clock_table_size > 0) {
		/* Reserved parents for all MUXes are at the end of the clock table.
		 * Search backwards from the end, matching both dev_id and clock_id.
		 * Stop early once we've checked n_reserved parents for this device.
		 */
		uint32_t checked = 0;

		for (long i = (long)clock_table_size - 1; i >= 0 && checked < n_reserved; i--) {
			parent = ti_scmi_id_to_clock((unsigned long)i);
			if (parent != NULL && parent->dev_id == clock->dev_id) {
				checked++;
				VERBOSE("Check reserved parent[%u/%u] at scmi_id=%ld clk_id=%u\n",
					checked, n_reserved, i, parent->clock_id);
				if (parent->clock_id == parent_id) {
					VERBOSE("Found reserved parent at scmi_id=%ld\n", i);
					return (int)i;
				}
			}
		}
	}

	ERROR("GET_PARENT: Parent not found for dev=%u clk=%u parent_id=%u\n",
	      clock->dev_id, clock->clock_id, parent_id);
	return -EINVAL;
}

static int ti_clk_set_parent(unsigned long scmi_id, unsigned long parent_scmi_id)
{
	struct ti_scmi_clock *clock = ti_scmi_id_to_clock(scmi_id);
	struct ti_scmi_clock *parent = ti_scmi_id_to_clock(parent_scmi_id);

	if (clock == NULL || parent == NULL || clock->dev_id != parent->dev_id) {
		return -EINVAL;
	}

	return ti_set_clock_parent_handler(clock->dev_id, clock->clock_id, parent->clock_id);
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
