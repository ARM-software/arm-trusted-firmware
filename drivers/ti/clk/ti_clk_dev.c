/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Device Clock Driver
 *
 * This driver provides clock nodes that are derived from device states,
 * allowing the clock framework to query and track device power states.
 * It implements a clock driver that reports frequency and state based on
 * the associated device's power state, enabling clock tree dependencies
 * on device power domains.
 */

#include <assert.h>

#include <ti_clk.h>
#include <ti_clk_dev.h>
#include <ti_container_of.h>
#include <ti_device.h>
#include <ti_device_clk.h>
#include <ti_device_pm.h>

/**
 * ti_clk_from_device_set_state() - Set the state of a device-derived clock.
 * @clkp: The clock instance (unused).
 * @enabled: True to enable, false to disable (unused).
 *
 * This clock's state is derived from the associated device's power domain
 * and cannot be directly controlled via the clock framework; the device
 * power state drives enablement, so always return success.
 *
 * Return: Always true.
 */
static bool ti_clk_from_device_set_state(struct ti_clk *clkp __maybe_unused,
					 bool enabled __maybe_unused)
{
	assert(clkp != NULL);

	return true;
}

static uint32_t ti_clk_from_device_get_state(struct ti_clk *clkp)
{
	const struct ti_clk_data_from_dev *from_device;
	struct ti_dev_clk *dev_clkp;
	struct ti_device *dev;
	uint32_t state;

	assert(clkp != NULL);

	from_device = ti_container_of((const struct ti_clk_drv_data *)clkp->data,
				      const struct ti_clk_data_from_dev, data);

	dev = ti_device_lookup(from_device->dev);
	if ((dev == NULL) || (dev->initialized == 0U)) {
		return TI_CLK_HW_STATE_DISABLED;
	}

	state = ti_device_get_state(dev);
	if (state == TI_DEVICE_STATE_DISABLED) {
		return TI_CLK_HW_STATE_DISABLED;
	}

	if (state == TI_DEVICE_STATE_TRANSITIONING) {
		return TI_CLK_HW_STATE_TRANSITION;
	}

	dev_clkp = ti_get_dev_clk(dev, from_device->clk_idx);
	if ((dev_clkp == NULL) || ((dev_clkp->flags & TI_DEV_CLK_FLAG_DISABLE) != 0U)) {
		return TI_CLK_HW_STATE_DISABLED;
	}

	return TI_CLK_HW_STATE_ENABLED;
}

const struct ti_clk_drv ti_clk_drv_from_device = {
	.get_freq = ti_clk_value_get_freq,
	.set_state = ti_clk_from_device_set_state,
	.get_state = ti_clk_from_device_get_state,
};
