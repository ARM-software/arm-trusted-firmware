/*
 * Copyright (c) 2020-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/mtk_init/mtk_init.h>
#include <mt_timer.h>
#include <platform_def.h>

uint64_t normal_time_base;
uint64_t atf_time_base;

void sched_clock_init(uint64_t normal_base, uint64_t atf_base)
{
	normal_time_base += normal_base;
	atf_time_base = atf_base;
}

uint64_t sched_clock(void)
{
	uint64_t cval;
	uint64_t rel_base;

	rel_base = read_cntpct_el0() - atf_time_base;
	cval = ((rel_base * 1000U) / SYS_COUNTER_FREQ_IN_MHZ)
		- normal_time_base;
	return cval;
}

int mt_systimer_init(void)
{
	INFO("[%s] systimer initialization\n", __func__);

	/* Enable access in NS mode */
	mmio_write_32(CNTWACR_REG, CNT_WRITE_ACCESS_CTL_MASK);
	mmio_write_32(CNTRACR_REG, CNT_READ_ACCESS_CTL_MASK);

	return 0;
}
MTK_PLAT_SETUP_0_INIT(mt_systimer_init);
