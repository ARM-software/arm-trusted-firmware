/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_dev.h>
#include <lib/container_of.h>
#include <device.h>
#include <device_pm.h>
#include <device_clk.h>
#include <clk.h>

static bool clk_from_device_notify_freq(struct clk	*clkp __attribute__(
						(unused)),
					uint32_t		parent_freq
					__attribute__(
						(unused)),
					bool		query __attribute__(
						(unused)))
{
	return false;
}

/* It might be useful to prevent the device from disabling in this case */
static bool clk_from_device_set_state(struct clk	*clkp __attribute__(
					      (unused)),
				      bool		enabled __attribute__(
					      (unused)))
{
	return true;
}

static uint32_t clk_from_device_get_state(struct clk *clkp)
{
	const struct clk_data *clk_datap = clk_get_data(clkp);
	const struct clk_data_from_dev *from_device;
	struct device *dev;
	uint32_t ret;

	from_device = container_of(clk_datap->data,
				   const struct clk_data_from_dev, data);

	dev = device_lookup(from_device->dev);

	if ((dev == NULL) || (dev->initialized == 0U)) {
		ret = CLK_HW_STATE_DISABLED;
	} else {
		uint32_t state;

		state = device_get_state(dev);
		if (state == 0U) {
			ret = CLK_HW_STATE_DISABLED;
		} else if (state == 2U) {
			ret = CLK_HW_STATE_TRANSITION;
		} else {
			struct dev_clk *dev_clkp;

			dev_clkp = get_dev_clk(dev, from_device->clk_idx);
			if (!dev_clkp) {
				ret = CLK_HW_STATE_DISABLED;
			} else if ((dev_clkp->flags & DEV_CLK_FLAG_DISABLE) != 0U) {
				ret = CLK_HW_STATE_DISABLED;
			} else {
				ret = CLK_HW_STATE_ENABLED;
			}
		}
	}

	return ret;
}

const struct clk_drv clk_drv_from_device = {
	.get_freq	= clk_value_get_freq,
	.notify_freq	= clk_from_device_notify_freq,
	.set_state	= clk_from_device_set_state,
	.get_state	= clk_from_device_get_state,
};
