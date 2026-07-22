/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TI_CLK_SOC_HFOSC0_H
#define TI_CLK_SOC_HFOSC0_H

#include <ti_clk.h>

/*
 * High Frequency Oscillator (HFOSC0) driver data.
 *
 * Holds board-specific register configuration used by the HFOSC0 driver
 * to read and validate the oscillator frequency.
 */
struct ti_clk_data_hfosc0 {
	/* Base driver data */
	struct ti_clk_drv_data data;
	/* Base address of WKUP_CTRL MMR containing HFOSC0 configuration */
	uint32_t wkup_ctrl_mmr_base;
	/* Offset to DEVSTAT register from wkup_ctrl_mmr_base */
	uint32_t devstat_offset;
	/* Bit mask for extracting frequency selection from DEVSTAT */
	uint32_t devstat_freq_mask;
	/* Supported frequencies in Hz, terminated with 0 */
	const uint32_t *supported_freqs;
};

extern const struct ti_clk_drv ti_clk_drv_soc_hfosc0;

#endif /* TI_CLK_SOC_HFOSC0_H */
