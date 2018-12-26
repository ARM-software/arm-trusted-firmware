/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include "micro_delay.h"

#define RCAR_CONV_MICROSEC		1000000U

void
#if IMAGE_BL31
	__attribute__ ((section (".system_ram")))
#endif
	rcar_micro_delay(uint64_t micro_sec)
{
	uint64_t freq;
	uint64_t base_count;
	uint64_t get_count;
	uint64_t wait_time = 0U;

	freq = read_cntfrq_el0();
	base_count = read_cntpct_el0();
	while (micro_sec > wait_time) {
		get_count = read_cntpct_el0();
		wait_time = ((get_count - base_count) * RCAR_CONV_MICROSEC) / freq;
	}
}
