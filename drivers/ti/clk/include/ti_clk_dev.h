/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Device Clock API Header
 *
 * This header defines the data structures and interfaces for clocks that
 * are derived from device power states. It allows the clock framework to
 * track device power domains as clock sources, providing frequency and
 * state information based on device power state.
 */

#ifndef TI_CLK_DEV_H
#define TI_CLK_DEV_H

#include <ti_clk.h>
#include <ti_device.h>
#include <ti_pm_types.h>

/*
 * Clock sourced from a device IP block
 *
 * Represents a SoC clock that derives its frequency and state from an IP
 * block's power domain. Allows the clock framework to query device power
 * states and clock outputs as clock sources.
 */
struct ti_clk_data_from_dev {
	/* Base clock driver data */
	struct ti_clk_drv_data data;
	/* Device index that provides this clock */
	ti_dev_idx_t dev;
	/* Clock index within the source device */
	ti_dev_clk_idx_t clk_idx;
};

/*
 * Device-sourced clock driver
 *
 * Implements clock operations for clocks derived from device IP blocks.
 * Queries device power state and output clocks to provide frequency and
 * enable information to the clock framework.
 */
extern const struct ti_clk_drv ti_clk_drv_from_device;

#endif /* TI_CLK_DEV_H */
