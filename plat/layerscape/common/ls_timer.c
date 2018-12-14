/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#define TIMER_BASE_ADDR 0x02B00000

uint64_t ls_get_timer(uint64_t start)
{
	return read_cntpct_el0() * 1000 / read_cntfrq_el0() - start;
}

static uint32_t ls_timeus_get_value(void)
{
	/*
	 * Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter. The value is
	 * clipped from 64 to 32 bits.
	 */
	return (uint32_t)(~read_cntpct_el0());
}

static const timer_ops_t ls_timer_ops = {
	.get_timer_value	= ls_timeus_get_value,
	.clk_mult		= 1,
	.clk_div		= 25,
};


/*
 * Initialise the nxp layerscape on-chip free rolling us counter as the delay
 * timer.
 */
void ls_delay_timer_init(void)
{
	uintptr_t cntcr =  TIMER_BASE_ADDR;

	mmio_write_32(cntcr, 0x1);

	timer_init(&ls_timer_ops);
}
