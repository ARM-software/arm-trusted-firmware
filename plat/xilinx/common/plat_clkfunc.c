/*
 * Copyright (c) 2023-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <common/debug.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <plat_private.h>

void set_cnt_freq(void)
{
	uint64_t counter_freq;

	/* Configure counter frequency */
	counter_freq = read_cntfrq_el0();
	if (counter_freq == 0U) {
		write_cntfrq_el0(plat_get_syscnt_freq2());
	}
}
