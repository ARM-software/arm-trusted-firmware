/*
 * Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <plat_clkfunc.h>
#include <plat_private.h>

uint32_t plat_get_syscnt_freq2(void)
{
	uint32_t counter_freq = 0;
	uint32_t ret = 0;

	counter_freq = mmio_read_32(IOU_SCNTRS_BASE +
				    IOU_SCNTRS_BASE_FREQ_OFFSET);
	if (counter_freq != 0U) {
		ret = counter_freq;
	} else {
		INFO("Indicates counter frequency %dHz setting to %dHz\n",
		     counter_freq, cpu_clock);
		ret = cpu_clock;
	}

	return ret;
}

void set_cnt_freq(void)
{
	uint64_t counter_freq;

	/* Configure counter frequency */
	counter_freq = read_cntfrq_el0();
	if (counter_freq == 0U) {
		write_cntfrq_el0(plat_get_syscnt_freq2());
	}
}
