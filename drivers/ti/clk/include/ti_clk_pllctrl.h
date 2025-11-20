/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI PLL Control API Header
 *
 * This header defines the interface for PLL control driver operations,
 * including the read-only PLL control mux driver for monitoring PLL lock
 * state and managing PLL bypass/lock-based clock path selection.
 */

#ifndef CLK_PLLCTRL_H
#define CLK_PLLCTRL_H

#include <ti_clk_mux.h>

/**
 * \brief Read-only PLL control mux driver
 *
 * Monitors PLL lock state and provides the current clock path selection
 * between bypass and locked modes. Does not support dynamic mux switching,
 * only reports the hardware-determined state based on PLL lock status.
 */
extern const struct ti_clk_drv_mux ti_clk_drv_pllctrl_mux_reg_ro;

#endif
