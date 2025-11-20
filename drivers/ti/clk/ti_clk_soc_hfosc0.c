/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI HFOSC0 (High Frequency Oscillator 0) Driver
 *
 * This driver provides software support for the system clock crystal,
 * which is the primary external oscillator providing the reference clock
 * for the entire SoC.
 */

#include <cdefs.h>

#include <ti_clk_soc_hfosc0.h> /* SoC-specific configuration */
#include <ti_io.h>
#include <ti_clk.h>

/*
 * System Oscillator enable control
 * HF Oscillator 0
 * Generic driver supporting: 19.2, 20, 24, 25, 26, 27 MHz
 */
static uint32_t ti_clk_soc_hfosc0_get_freq(struct ti_clk *clk_ptr __unused)
{
	uint32_t reg_val;
	uint32_t freq_from_reg;
	uint32_t i;

	const uint32_t freq_table[] = {
		FREQ_MHZ(19.2),
		FREQ_MHZ(20.0),
		FREQ_MHZ(24.0),
		FREQ_MHZ(25.0),
		FREQ_MHZ(26.0),
		FREQ_MHZ(27.0),
	};

	/* SoC-specific supported frequencies */
	const uint32_t supported_freqs[] = HFOSC0_SUPPORTED_FREQS;

	reg_val = readl(HFOSC0_WKUP_CTRL_MMR_BASE + HFOSC0_MAIN_DEVSTAT_OFFSET)
		& HFOSC0_DEVSTAT_FREQ_MASK;

	if (reg_val >= ARRAY_SIZE(freq_table)) {
		return 0;
	}

	freq_from_reg = freq_table[reg_val];

	/* Check if frequency is in SoC's supported list */
	for (i = 0U; supported_freqs[i] != 0U; i++) {
		if (freq_from_reg == supported_freqs[i]) {
			return freq_from_reg;
		}
	}

	return 0;
}

static uint32_t ti_clk_soc_hfosc0_get_state(struct ti_clk *clk_ptr __unused)
{
	return TI_CLK_HW_STATE_ENABLED;
}

const struct ti_clk_drv ti_clk_drv_soc_hfosc0 = {
	.get_freq = ti_clk_soc_hfosc0_get_freq,
	.get_state = ti_clk_soc_hfosc0_get_state,
};
