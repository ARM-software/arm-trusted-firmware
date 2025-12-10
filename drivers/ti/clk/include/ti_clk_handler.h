/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI Clock Handler API Header
 *
 * This header defines the SCMI clock protocol handler interface for TI SoCs.
 * It provides the API functions called by the SCMI layer to handle clock
 * operations from the host, including initialization and protocol handler
 * registration.
 */

#ifndef CLK_HANDLER_H
#define CLK_HANDLER_H

/**
 * The IP does not require this clock, it can be disabled, regar`dless of the
 * state of the device
 */
#define CLOCK_SW_STATE_UNREQ	    0

/**
 * Allow the system controller to automatically manage the state of this clock.
 * If the device is enabled, then the clock is enabled. If the device is set to
 * off or retention, then the clock is internally set as not being required
 * by the device. This is the default state.
 */
#define CLOCK_SW_STATE_AUTO	    1

/**
 *  \brief  Initialize the clock handler and register clk_ops
 *
 *  This function registers the TI clock operations with the generic
 *  clock framework. Must be called before using the clock API.
 */
void ti_clk_handler_init(void);

#endif /* CLK_HANDLER_H */
