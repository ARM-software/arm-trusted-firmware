/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Power Management
 *
 * This module handles the main device power management operations including
 * device enable/disable, state management, retention control, and suspend handling.
 */

#include <assert.h>

#include <ti_device.h>
#include <ti_device_clk.h>
#include <ti_device_pm.h>

/**
 * device_enable() - Enable a device.
 * @dev: The device to enable.
 *
 * Performs the steps necessary to enable a device.
 */
static void device_enable(struct ti_device *dev)
{
	const struct ti_dev_data *data;
	uint16_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	for (i = 0U; i < data->n_clocks; i++) {
		ti_device_clk_enable(dev, i);
	}
	ti_soc_device_enable(dev);
}

/**
 * ti_device_disable() - Disables a device.
 * @device: The device to modify.
 * @domain_reset: True if the device is being disabled due to a domain reset.
 *
 * Performs the steps necessary to disable a device.
 */
void ti_device_disable(struct ti_device *dev, bool domain_reset)
{
	const struct ti_dev_data *data;
	int32_t i;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	ti_soc_device_disable(dev, domain_reset);

	for (i = (int32_t) data->n_clocks - 1; i >= 0; i--) {
		ti_device_clk_disable(dev, (uint16_t) i);
	}
}

/**
 * ti_device_clear_flags() - Clear device initialization flags.
 * @dev: The device to modify.
 */
void ti_device_clear_flags(struct ti_device *dev)
{
	assert(dev != NULL);

	ti_soc_device_clear_flags(dev);
}

/**
 * ti_device_set_state() - Set device enabled state.
 * @device_ptr: The device to modify.
 * @host_idx: Index of the host making the request.
 * @enable: True to enable the device, false to disable.
 */
void ti_device_set_state(struct ti_device *device_ptr, uint8_t host_idx, bool enable)
{
	bool was_enabled;
	bool is_enabled;

	assert(device_ptr != NULL);
	assert((TI_DEV_FLAG_ENABLED(host_idx) & TI_DEV_FLAG_ENABLED_MASK) != 0UL);

	was_enabled = (device_ptr->flags & TI_DEV_FLAG_ENABLED_MASK) != 0UL;

	if (enable) {
		device_ptr->flags |= (uint32_t)TI_DEV_FLAG_ENABLED(host_idx);
	} else {
		device_ptr->flags &= ~(uint32_t)TI_DEV_FLAG_ENABLED(host_idx);
	}

	/*
	 * As soon as any host gets or puts a device, we drop the power
	 * on enabled flag.
	 */
	if (host_idx != TI_DEV_POWER_ON_ENABLED_HOST_IDX) {
		device_ptr->flags &= ~(uint32_t)TI_DEV_FLAG_POWER_ON_ENABLED;
	}

	is_enabled = (device_ptr->flags & TI_DEV_FLAG_ENABLED_MASK) != 0UL;
	if (was_enabled != is_enabled) {
		if (is_enabled) {
			device_enable(device_ptr);
		} else {
			ti_device_disable(device_ptr, false);
		}
	}
}

/**
 * ti_device_set_retention() - Enable or disable device retention.
 * @device_ptr: The device to modify.
 * @retention: True to enable retention, false to disable.
 */
void ti_device_set_retention(struct ti_device *device_ptr, bool retention)
{
	bool is_retention;

	assert(device_ptr != NULL);

	is_retention = ((device_ptr->flags & TI_DEV_FLAG_RETENTION) != 0U);

	if (retention == is_retention) {
		return;
	}

	if (retention) {
		device_ptr->flags |= (uint32_t)TI_DEV_FLAG_RETENTION;
		ti_soc_device_ret_enable(device_ptr);
	} else {
		device_ptr->flags &= ~(uint32_t)TI_DEV_FLAG_RETENTION;
		ti_soc_device_ret_disable(device_ptr);
	}
}

/**
 * ti_device_id_disable_clocks() - Disable all clocks for a device
 * @idx: The device index
 */
void ti_device_id_disable_clocks(ti_dev_idx_t idx)
{
	struct ti_device *dev;
	const struct ti_dev_data *data;
	int32_t i;

	dev = ti_device_lookup(idx);

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	for (i = (int32_t)data->n_clocks - 1; i >= 0; i--) {
		ti_device_clk_disable(dev, (uint16_t)i);
	}
}

/**
 * ti_device_id_power_down() - Power down a device without touching clocks
 * @idx: The device index
 */
void ti_device_id_power_down(ti_dev_idx_t idx)
{
	struct ti_device *dev;

	dev = ti_device_lookup(idx);

	assert(dev != NULL);

	dev->flags &= ~(uint32_t)TI_DEV_FLAG_POWER_ON_ENABLED;

	ti_soc_device_disable(dev, false);

	ti_device_set_retention(dev, false);
}

/**
 * ti_device_id_enable_clocks() - Enable all clocks for a device
 * @idx: The device index
 *
 * Enables all clocks associated with the specified device.
 */
void ti_device_id_enable_clocks(ti_dev_idx_t idx)
{
	struct ti_device *dev;
	const struct ti_dev_data *data;
	uint16_t i;

	dev = ti_device_lookup(idx);

	assert(dev != NULL);

	data = ti_get_dev_data(dev);

	for (i = 0U; i < data->n_clocks; i++) {
		ti_device_clk_enable(dev, i);
	}
}

/**
 * ti_device_id_power_up() - Power up a device without touching clocks
 * @idx: The device index
 *
 * Powers up the specified device without touching clocks.
 */
void ti_device_id_power_up(ti_dev_idx_t idx)
{
	struct ti_device *dev;

	dev = ti_device_lookup(idx);

	assert(dev != NULL);

	ti_device_set_retention(dev, true);

	dev->flags |= (uint32_t)TI_DEV_FLAG_POWER_ON_ENABLED;

	ti_soc_device_enable(dev);
}
