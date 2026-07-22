/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Initialization and Management
 *
 * This module provides core device initialization, deferred initialization
 * support, and device lifecycle management for the device power management subsystem.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include <common/debug.h>
#include <ti_clk.h>
#include <ti_device.h>
#include <ti_device_clk.h>
#include <ti_device_pm.h>

/* True if deferred (read-write) initialization has been initiated. */
static bool devices_rw;

bool ti_pm_devgroup_is_enabled(devgrp_t groups)
{
	return true;
}

/**
 * device_init() - Initialize a device
 * @dev: The device to init.
 *
 * This performs the necessary device initialization step, including syncing
 * the flags on the device with the real hardware state and calling the clock
 * init function for each clock.
 *
 * Return: 0 on success, <0 on failure.
 */
static int32_t device_init(struct ti_device *dev)
{
	const struct ti_dev_data *data;
	const struct ti_drv *drvp = NULL;
	uint16_t i;
	int32_t ret;

	assert(dev != NULL);

	data = ti_get_dev_data(dev);
	if (data == NULL) {
		return 0;
	}

	if (((data->flags & TI_DEVD_FLAG_DRV_DATA) != 0U) &&
	    ((data->flags & TI_DEVD_FLAG_DO_INIT) != 0U)) {
		drvp = ti_to_drv_data(data)->drv;
	}

	if ((drvp != NULL) && (drvp->pre_init != NULL)) {
		ret = drvp->pre_init(dev);
		if (ret != 0) {
			return ret;
		}
	}

	if (devices_rw == false) {
		/* Defer remainder of init */
		return 0;
	}

	ret = ti_soc_device_init(dev);
	if (ret != 0) {
		return ret;
	}

	for (i = 0U; i < data->n_clocks; i++) {
		ti_device_clk_init(dev, i);
	}

	/* Calling these multiple times for a deferred device has no effect */
	if (ti_device_get_state(dev) != 0U) {
		ti_device_set_state(dev, TI_DEV_POWER_ON_ENABLED_HOST_IDX, true);
		ti_device_set_retention(dev, true);
	}

	if ((drvp != NULL) && (drvp->post_init != NULL)) {
		ret = drvp->post_init(dev);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

int32_t ti_devices_init(void)
{
	bool done;
	bool progress;
	bool contents;
	ti_dev_idx_t idx;

	contents = false;
	do {
		struct ti_device *dev;

		done = true;
		progress = false;

		for (idx = 0U; idx < soc_device_count; idx++) {
			devgrp_t devgrp;
			int32_t ret;

			dev = &soc_devices[idx];
			if (dev->initialized != 0U) {
				continue;
			}

			if (soc_device_data_arr[idx] == NULL) {
				continue;
			}

			/* Translate compressed internal representation to bitfield */
			devgrp = (devgrp_t) BIT(soc_device_data_arr[idx]->pm_devgrp - 1U);

			if (ti_pm_devgroup_is_enabled(devgrp) == false) {
				continue;
			}

			contents = true;

			ret = device_init(dev);
			if (ret == -EAGAIN) {
				done = false;
				continue;
			}

			if (ret < 0) {
				VERBOSE("ACTION FAIL: DEV_INIT dev_id=%d error=%d\n",
					idx, -ret);
			}

			progress = true;
			dev->initialized = 1U;
		}
	} while (!done && progress);

	if (devices_rw == true) {
		/* Only necessary after deferred initialization */
		ti_clk_drop_pwr_up_en();
	}

	if ((progress == false) && (contents == true)) {
		/* We processed at least one device but didn't make progress */
		return -EAGAIN;
	}

	return 0;
}

int32_t ti_devices_init_rw(void)
{
	if (devices_rw == false) {
		uint32_t i;

		/*
		 * Force reinitialization of all devices to get deferred
		 * initialization.
		 */
		for (i = 0U; i < soc_device_count; i++) {
			struct ti_device *dev = &soc_devices[i];

			dev->initialized = 0U;
		}

		devices_rw = true;

		/* Perform deferred initialization */
		return ti_devices_init();
	}

	return 0;
}
