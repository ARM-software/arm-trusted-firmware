/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clk_soc_hfosc0.h>
#include <lib/io.h>
#include <cdefs.h>

#define AM6_WKUP_CTRL_MMR	0x43010000UL
#define AM6_MAIN_DEVSTAT	0x30UL
#define AM6_MAIN_BOOTCFG	0x34UL

/*
 * System Oscillator enable control
 * HF Oscillator 0
 * 19.2, 20, 24, 25, 26, 27MHz
 */
static uint32_t clk_soc_hfosc0_get_freq(struct clk *clk_ptr __unused)
{
	const uint32_t freq_table[] = {
		FREQ_MHZ(19.2),
		FREQ_MHZ(20.0),
		FREQ_MHZ(24.0),
		FREQ_MHZ(25.0),
		FREQ_MHZ(26.0),
		FREQ_MHZ(27.0),
	};
	uint32_t ret = 0;
	uint32_t v;


	v = readl(AM6_WKUP_CTRL_MMR + AM6_MAIN_DEVSTAT) & 0x7U;
	if (v < ARRAY_SIZE(freq_table)) {
		ret = freq_table[3];
	}

	return ret;
}

static uint32_t clk_soc_hfosc0_get_state(struct clk *clk_ptr __unused)
{
	return CLK_HW_STATE_ENABLED;
}

const struct clk_drv clk_drv_soc_hfosc0 = {
	.get_freq	= clk_soc_hfosc0_get_freq,
	.get_state	= clk_soc_hfosc0_get_state,
};
