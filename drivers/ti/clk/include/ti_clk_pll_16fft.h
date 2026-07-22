/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI 16FFT PLL API Header
 *
 * This header defines the 16FFT (16-bit Fractional Feedback) PLL specific
 * data structures and driver interface. It provides the register layouts,
 * HSDIV configuration, and driver operations for TI's fractional-N PLL
 * variant supporting VCO frequencies from 800 MHz to 3.2 GHz.
 */

#ifndef TI_CLK_PLL_16FFT_H
#define TI_CLK_PLL_16FFT_H

#include <ti_clk_pll.h>

/*
 * 16FFT PLL clock driver data
 *
 * Extends the base PLL data structure with 16FFT-specific register base
 * address and HSDIV output index. Used by the 16FFT PLL driver to manage
 * fractional-N PLL instances and their high-speed divider outputs.
 */
struct ti_clk_data_pll_16fft {
	/* Base PLL driver data (frequency, dividers, etc.) */
	struct ti_clk_data_pll data_pll;
	/* Physical base address of the PLL register block */
	uint32_t base;
	/* HSDIV output index (0-15) for this clock instance */
	uint8_t idx;
};

/*
 * Main 16FFT PLL clock driver
 *
 * Provides clock operations for the 16FFT PLL core including initialization,
 * frequency query, VCO configuration, and lock management.
 */
extern const struct ti_clk_drv ti_clk_drv_pll_16fft;

/*
 * 16FFT PLL post-divider driver
 *
 * Manages the programmable post-divider stage that divides the VCO output
 * before distribution. Supports divide ratios from 1 to 16.
 */
extern const struct ti_clk_drv_div ti_clk_drv_div_pll_16fft_postdiv;

/*
 * 16FFT PLL HSDIV output driver
 *
 * Controls individual high-speed divider outputs (HSDIV0-HSDIV15) that
 * divide the post-divided PLL output. Each HSDIV supports independent
 * divide ratios from 1 to 128.
 */
extern const struct ti_clk_drv_div ti_clk_drv_div_pll_16fft_hsdiv;

/*
 * 16FFT PLL combined post-divider and HSDIV driver
 *
 * Composite driver managing both the post-divider and HSDIV stages as a
 * single clock node. Used for clocks that control the complete divider chain
 * from VCO to final output.
 */
extern const struct ti_clk_drv_div ti_clk_drv_div_pll_16fft_postdiv_hsdiv;

#endif /* TI_CLK_PLL_16FFT_H */
