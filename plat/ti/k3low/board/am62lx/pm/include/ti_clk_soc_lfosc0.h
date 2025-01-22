/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_CLK_AM62_LFOSC_H
#define SOC_CLK_AM62_LFOSC_H

#include <ti_clk.h>

/* AM62LX board-specific configuration */
#define LFOSC0_RTC_BASE           0x2b1f0000UL
#define LFOSC0_LFXOSC_CTRL_OFFSET 0x80UL
#define LFOSC0_32K_DISABLE_VAL    0x80UL
#define LFOSC0_FREQ_HZ            32768U

extern const struct ti_clk_drv ti_clk_drv_soc_lfosc0;

#endif
