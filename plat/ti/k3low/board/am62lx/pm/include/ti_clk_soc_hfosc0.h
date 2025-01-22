/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_CLK_AM6_H
#define SOC_CLK_AM6_H

#include <cdefs.h>

#include <ti_clk.h>

/* AM62LX board-specific configuration */
#define HFOSC0_WKUP_CTRL_MMR_BASE    0x43010000UL
#define HFOSC0_MAIN_DEVSTAT_OFFSET   0x30UL
#define HFOSC0_DEVSTAT_FREQ_MASK     0x7U

/*
 * AM62LX supported HFOSC0 frequencies
 * List all frequencies this board can support (terminated with 0)
 */
#define HFOSC0_SUPPORTED_FREQS { FREQ_MHZ(25.0), 0U }

extern const struct ti_clk_drv ti_clk_drv_soc_hfosc0;

#endif
