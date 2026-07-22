/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TI_CLK_SOC_LFOSC0_H
#define TI_CLK_SOC_LFOSC0_H

#include <ti_clk.h>

/*
 * Low Frequency Oscillator (LFOSC0) driver data.
 *
 * Holds board-specific register configuration used by the LFOSC0 driver
 * to read the oscillator control register and determine its state.
 */
struct ti_clk_data_lfosc0 {
	/* Base driver data */
	struct ti_clk_drv_data data;
	/* Base address of RTC module containing LFOSC0 control registers */
	uint32_t rtc_base;
	/* Offset to LFXOSC control register from rtc_base */
	uint32_t lfxosc_ctrl_offset;
	/* Bit mask for LFOSC0 32K disable bit in control register */
	uint32_t disable_mask;
	/* Fixed frequency of LFOSC0 in Hz */
	uint32_t freq_hz;
};

extern const struct ti_clk_drv ti_clk_drv_soc_lfosc0;

#endif /* TI_CLK_SOC_LFOSC0_H */
