/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_soc_lfosc0.h>
#include <lib/io.h>
#include <cdefs.h>

#define AM62L_WKUP_RTCSS0_RTC_BASE    0x2b1f0000UL
#define AM62L_WKUP_RTCSS0_LFXOSC_CTRL 0x80UL
#define LFXOSC_32K_DISABLE_VAL	      0x80UL

/*
 * LFXOSC_CTRL - Low Frequency Oscillator Control Register
 * 32.768 KHz
 */
static uint32_t clk_soc_lfosc0_get_freq(struct clk *clk_ptr __unused)
{
	uint32_t ret = 0;
	uint32_t v;

	v = readl(AM62L_WKUP_RTCSS0_RTC_BASE + AM62L_WKUP_RTCSS0_LFXOSC_CTRL)
		& LFXOSC_32K_DISABLE_VAL;
	if (v == 0) {
		ret = FREQ_KHZ(32.768);
	}

	return ret;
}

static uint32_t clk_soc_lfosc0_get_state(struct clk *clk_ptr __unused)
{
	return CLK_HW_STATE_ENABLED;
}

const struct clk_drv clk_drv_soc_lfosc0 = {
	.get_freq	= clk_soc_lfosc0_get_freq,
	.get_state	= clk_soc_lfosc0_get_state,
};
