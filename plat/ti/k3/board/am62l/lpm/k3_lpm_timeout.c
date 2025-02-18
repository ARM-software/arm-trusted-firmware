/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <k3_lpm_timeout.h>
#include <plat/common/platform.h>

__wkupsramfunc void k3_lpm_delay_1us(void)
{
	uint32_t tick_start = (uint32_t)read_cntpct_el0();
	uint32_t us_ticks = (SYS_COUNTER_FREQ_IN_TICKS / 1000000);

	while (((uint32_t)read_cntpct_el0() - tick_start) < us_ticks) {
	}
}
