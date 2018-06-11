/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <stdio.h>
#include <thunder_private.h>
#include <delay_timer.h>
#include <platform.h>

#undef DEBUG_TIMERS

#ifdef DEBUG_TIMERS
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...)
#endif

/* max period is over 18e10 seconds */
static void cntps_set_period(uint64_t period)
{
	uint64_t timer_status;

	/* read current timer counter value */
	__asm__ volatile ("mrs %[stat], cntpct_el0" : [stat] "=r" (timer_status));

	period += timer_status;

	__asm__ volatile ("msr cntps_cval_el1, %[period]" :: [period] "r" (period));
}

static uint32_t plat_get_timer_value(void)
{
	/* Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter. */
	volatile uint64_t count = CSR_READ_PA(0, CAVM_RST_REF_CNTR);

	return ~count;
}

static timer_ops_t plat_timer_ops;

int thunder_timers_init(void)
{
	/* Program the counter frequency */
	write_cntfrq_el0(plat_get_syscnt_freq2());
	cntps_set_period(~0);

	/* enable timer with unmasked interrupts */
	uint32_t ctl = 0x3;

	/* write ctl value */
	__asm__ volatile ("msr cntps_ctl_el1, %[ctl]" :: [ctl] "r" (ctl));

	plat_timer_ops.get_timer_value	= plat_get_timer_value;
	plat_timer_ops.clk_mult		= 1;
	plat_timer_ops.clk_div		= THUNDER_SYSCNT_FREQ / 2;

	timer_init(&plat_timer_ops);

	/* return number of available hw counters */
	return 1;
}
