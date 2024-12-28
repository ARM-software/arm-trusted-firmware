/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLK_DEV_H
#define CLK_DEV_H

#include <clk.h>
#include <device.h>
#include <pm_types.h>

/* SoC clock sourced from an IP block */
struct clk_data_from_dev {
	struct clk_drv_data	data;
	dev_idx_t		dev;
	dev_clk_idx_t		clk_idx;
};

extern const struct clk_drv clk_drv_from_device;

#endif
