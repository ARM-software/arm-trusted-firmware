/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Fixed Clock API Header
 *
 * This header defines the interface for fixed-frequency clock drivers.
 * Fixed clocks provide unchangeable frequencies, are always enabled, and
 * are typically used for board-level reference clocks.
 */

#ifndef CLK_FIXED_H
#define CLK_FIXED_H

#include <ti_clk.h>

/**
 * \brief Fixed-frequency clock driver
 *
 * Provides clock operations for fixed, unchangeable frequency clocks such as
 * crystal oscillators and board-level reference clocks. These clocks are
 * always enabled and report a constant frequency.
 */
extern const struct ti_clk_drv ti_clk_drv_fixed;

#endif
