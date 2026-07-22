/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
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

#ifndef TI_CLK_HANDLER_H
#define TI_CLK_HANDLER_H

/*
 * ti_clk_handler_init() - Initialize the clock handler and register clk_ops
 *
 * Registers the TI clock operations with the generic clock framework.
 * Must be called before using the clock API.
 */
void ti_clk_handler_init(void);

#endif /* TI_CLK_HANDLER_H */
