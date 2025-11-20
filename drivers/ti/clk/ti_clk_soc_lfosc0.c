/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * TI LFOSC0 (Low Frequency Oscillator 0) Driver
 *
 * This driver provides software support for the low frequency oscillator,
 * which is typically a 32 KHz crystal used for RTC (Real-Time Clock) and
 * low-power timekeeping.
 */

#include <cdefs.h>

#include <ti_clk_soc_lfosc0.h> /* SoC-specific configuration */
#include <ti_io.h>
#include <ti_clk.h>

/*
 * System Oscillator enable control
 * LF Oscillator 0
 * Generic driver supporting: 32 KHz
 */
static uint32_t ti_clk_soc_lfosc0_get_freq(struct ti_clk *clk_ptr __unused)
{
	uint32_t reg_val;

	reg_val = (uint32_t)(readl(LFOSC0_RTC_BASE + LFOSC0_LFXOSC_CTRL_OFFSET)
		& LFOSC0_32K_DISABLE_VAL);
	if (reg_val == 0U) {
		return LFOSC0_FREQ_HZ;
	}

	return 0U;
}

static uint32_t ti_clk_soc_lfosc0_get_state(struct ti_clk *clk_ptr __unused)
{
	return TI_CLK_HW_STATE_ENABLED;
}

const struct ti_clk_drv ti_clk_drv_soc_lfosc0 = {
	.get_freq = ti_clk_soc_lfosc0_get_freq,
	.get_state = ti_clk_soc_lfosc0_get_state,
};
