/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Device Initialization and Management
 *
 * This module provides core device initialization, deferred initialization
 * support, and device lifecycle management for the device power management subsystem.
 */

#include <stddef.h>
#include <errno.h>

#include <common/debug.h>

#include <ti_clk.h>
#include <ti_device.h>
#include <ti_device_clk.h>
#include <ti_device_pm.h>

/** True if devices have completed deferred init. */
static bool devices_rw;

bool ti_pm_devgroup_is_enabled(devgrp_t groups)
{
	return true;
}

/**
 * \brief Initialize a device
 *
 * This performs the necessary device initialization step, including syncing
 * the flags on the device with the real hardware state and calling the clock
 * init function for each clock.
 *
 * \param device
 * The device to init.
 *
 * \return
 * 0 on success, <0 on failure.
 */
static int32_t device_init(struct ti_device *dev)
{
	const struct ti_dev_data *data = get_dev_data(dev);
	uint16_t i;
	int32_t ret;

	if ((data != NULL) && ((data->flags & DEVD_FLAG_DRV_DATA) != 0U) &&
	    ((data->flags & DEVD_FLAG_DO_INIT) != 0U)) {
		const struct ti_drv *drvp = to_drv_data(data)->drv;

		if ((drvp != NULL) && (drvp->pre_init != NULL)) {
			ret = drvp->pre_init(dev);
			if (ret != 0) {
				return ret;
			}
		}
	}

	if (!devices_rw) {
		/* Defer remainder of init */
		data = NULL;
	}

	if (data != NULL) {
		ret = ti_soc_device_init(dev);
		if (ret != 0) {
			return ret;
		}
	}

	/* Calling these multiple times for a deferred device has no effect */
	if (data != NULL) {
		for (i = 0U; i < data->n_clocks; i++) {
			ti_device_clk_init(dev, i);
		}

		/* Calling these multiple times for a deferred device has no effect */
		if (device_get_state(dev) != 0U) {
			ti_device_set_state(dev, DEV_POWER_ON_ENABLED_HOST_IDX, true);
			ti_device_set_retention(dev, true);
		}

		if (((data->flags & DEVD_FLAG_DRV_DATA) != 0U) &&
		    ((data->flags & DEVD_FLAG_DO_INIT) != 0U)) {
			const struct ti_drv *drvp = to_drv_data(data)->drv;

			if ((drvp != NULL) && (drvp->post_init != NULL)) {
				ret = drvp->post_init(dev);
				if (ret != 0) {
					return ret;
				}
			}
		}
	}

	return 0;
}

int32_t ti_devices_init(void)
{
	bool done;
	bool progress;
	bool contents;
	bool errors;
	dev_idx_t idx;

	errors = false;
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

			if (!ti_pm_devgroup_is_enabled(devgrp)) {
				continue;
			}

			contents = true;

			ret = device_init(dev);
			if (ret == -EAGAIN) {
				done = false;
			} else {
				progress = true;
				dev->initialized = 1U;
				if (ret < 0) {
					VERBOSE("ACTION FAIL: DEV_INIT dev_id=%d error=%d\n",
						idx, -ret);
					errors = true;
				}
			}
		}
	} while (!done && progress);

	if (devices_rw) {
		/* Only necessary after deferred initialization */
		ti_clk_drop_pwr_up_en();
	}

	if (progress) {
		if (devices_rw) {
			/* Only necessary after deferred initialization */
			/* ti_soc_device_init_complete(); */
		}

		if (errors == false) {
			VERBOSE("DEV_INIT\n");
		}

		return 0;
	}

	if (contents) {
		/* We processed at least one device but didn't make progress */
		return -EAGAIN;
	}

	/* We didn't process any devices */
	return 0;
}

int32_t ti_devices_init_rw(void)
{
	if (!devices_rw) {
		uint32_t i;

		/*
		 * Force reinitialization of all devices to get defered
		 * initialization.
		 */
		for (i = 0U; i < soc_device_count; i++) {
			struct ti_device *dev = &soc_devices[i];

			dev->initialized = 0U;
		}

		devices_rw = true;

		/* Perform defered initialization */
		return ti_devices_init();
	}

	return 0;
}
