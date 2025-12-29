/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Power Management
 *
 * This module handles the main device power management operations including
 * device enable/disable, state management, retention control, and suspend handling.
 */

#include <common/debug.h>

#include <ti_device.h>
#include <ti_device_clk.h>
#include <ti_device_pm.h>

void ti_device_suspend(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);

	if ((data->flags & DEVD_FLAG_DRV_DATA) != 0U) {
		const struct ti_drv *drvp = to_drv_data(data)->drv;

		if (drvp && drvp->suspend) {
			drvp->suspend(dev);
		}
	}
}

/**
 * \brief Enables a device.
 *
 * Performs the steps necessary to enable a device.
 *
 * \param device
 * The device to modify.
 */
static void device_enable(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);
	uint16_t i;

	for (i = 0U; i < data->n_clocks; i++) {
		ti_device_clk_enable(dev, i);
	}
	ti_soc_device_enable(dev);
}

/**
 * \brief Disables a device.
 *
 * Performs the steps necessary to disable a device.
 *
 * \param device
 * The device to modify.
 * \param domain_reset
 * True if the device is being disabled due to a domain reset.
 */
void ti_device_disable(struct ti_device *dev, bool domain_reset)
{
	const struct ti_dev_data *data = get_dev_data(dev);
	int32_t i;

	ti_soc_device_disable(dev, domain_reset);

	for (i = (int32_t) data->n_clocks - 1; i >= 0; i--) {
		ti_device_clk_disable(dev, (uint16_t) i);
	}
}

void ti_device_clear_flags(struct ti_device *dev)
{
	ti_soc_device_clear_flags(dev);
}

void ti_device_set_state(struct ti_device *device_ptr, uint8_t host_idx, bool enable)
{
	bool was_enabled;
	bool is_enabled;

	was_enabled = (device_ptr->flags & DEV_FLAG_ENABLED_MASK) != 0UL;

	if (enable) {
		device_ptr->flags |= (uint32_t)DEV_FLAG_ENABLED(host_idx);
	} else {
		device_ptr->flags &= ~(uint32_t)DEV_FLAG_ENABLED(host_idx);
	}

	/*
	 * As soon as any host gets or puts a device, we drop the power
	 * on enabled flag.
	 */
	if (host_idx != DEV_POWER_ON_ENABLED_HOST_IDX) {
		device_ptr->flags &= ~(uint32_t)DEV_FLAG_POWER_ON_ENABLED;
	}

	is_enabled = (device_ptr->flags & DEV_FLAG_ENABLED_MASK) != 0UL;
	if (was_enabled != is_enabled) {
		if (is_enabled) {
			device_enable(device_ptr);
		} else {
			ti_device_disable(device_ptr, false);
		}
	}
}

void ti_device_set_retention(struct ti_device *device_ptr, bool retention)
{
	bool is_retention = ((device_ptr->flags & DEV_FLAG_RETENTION) != 0U);

	if (retention == is_retention) {
		/* Do nothing  - return */
	} else {
		if (retention) {
			device_ptr->flags |= DEV_FLAG_RETENTION;
			ti_soc_device_ret_enable(device_ptr);
		} else {
			device_ptr->flags &= ~(uint32_t)DEV_FLAG_RETENTION;
			ti_soc_device_ret_disable(device_ptr);
		}
	}
}
