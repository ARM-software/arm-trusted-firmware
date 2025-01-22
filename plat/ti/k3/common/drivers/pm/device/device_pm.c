/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <types/errno.h>
#include <device.h>
#include <device_pm.h>
#include <device_clk.h>
#include <lib/trace.h>
#include <list.h>
#include <notifier.h>

#include <common/debug.h>
static LIST_HEAD(device_notifiers);

static void device_notify(struct device *dev)
{
	notifier_call(&device_notifiers, dev);
}

void register_device_notifier(struct notifier *n)
{
	notifier_register(&device_notifiers, n);
}

void device_suspend(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);

	if ((data->flags & DEVD_FLAG_DRV_DATA) != 0U) {
		const struct drv *drvp = to_drv_data(data)->drv;

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
static void device_enable(struct device *dev)
{
	const struct dev_data *data = get_dev_data(dev);
	uint16_t i;

	for (i = 0U; i < data->n_clocks; i++) {
		/* FIXME: Error handling */
		device_clk_enable(dev, i);
	}
	soc_device_enable(dev);

	device_notify(dev);
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
void device_disable(struct device *dev, bool domain_reset)
{
	const struct dev_data *data = get_dev_data(dev);
	int32_t i;

	device_notify(dev);

	soc_device_disable(dev, domain_reset);

	for (i = (int32_t) data->n_clocks - 1; i >= 0; i--) {
		device_clk_disable(dev, (uint16_t) i);
	}
}

void device_clear_flags(struct device *dev)
{
	device_notify(dev);

	soc_device_clear_flags(dev);
}

void device_set_state(struct device *device_ptr, uint8_t host_idx, bool enable)
{
	bool was_enabled;
	bool is_enabled;

	was_enabled = (device_ptr->flags & DEV_FLAG_ENABLED_MASK) != 0UL;

	if (enable) {
		device_ptr->flags |= DEV_FLAG_ENABLED(host_idx);
	} else {
		device_ptr->flags &= ~DEV_FLAG_ENABLED(host_idx);
	}

	/*
	 * As soon as any host gets or puts a device, we drop the power
	 * on enabled flag.
	 */
	if (host_idx != DEV_POWER_ON_ENABLED_HOST_IDX) {
		device_ptr->flags &= ~DEV_FLAG_POWER_ON_ENABLED;
	}

	is_enabled = (device_ptr->flags & DEV_FLAG_ENABLED_MASK) != 0UL;
	if (was_enabled != is_enabled) {
		if (is_enabled) {
			device_enable(device_ptr);
		} else {
			device_disable(device_ptr, false);
		}
	}
}

void device_set_retention(struct device *device_ptr, bool retention)
{
	bool is_retention = (bool) !!(device_ptr->flags & DEV_FLAG_RETENTION);

	if (retention == is_retention) {
		/* Do nothing  - return */
	} else {
		if (retention) {
			device_ptr->flags |= DEV_FLAG_RETENTION;
			soc_device_ret_enable(device_ptr);
		} else {
			device_ptr->flags &= ~DEV_FLAG_RETENTION;
			soc_device_ret_disable(device_ptr);
		}
	}
}
