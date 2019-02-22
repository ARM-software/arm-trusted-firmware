/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

/* Ticks elapsed in one second by a signal of 1 MHz */
#define MHZ_TICKS_PER_SEC 1000000
/* Ticks elapsed in one second by a signal of 1 KHz */
#define KHZ_TICKS_PER_SEC 1000

static timer_ops_t ops;

uint64_t get_timer_val(uint64_t start)
{
	return (read_cntpct_el0() * 1000 / read_cntfrq_el0() - start);
}

static uint32_t timer_get_value(void)
{
	uint64_t cntpct = read_cntpct_el0();
#ifdef ERRATA_LS_A008585
	uint8_t	max_fetch_count = 10;
	/* This erratum number needs to be confirmed to match ARM document */
	uint64_t temp = read_cntpct_el0();

	while (temp != cntpct && max_fetch_count) {
		cntpct = read_cntpct_el0();
		temp = read_cntpct_el0();
		max_fetch_count--;
	}
#endif

	/*
	 * Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter. The value is
	 * clipped from 64 to 32 bits.
	 */
	return (uint32_t)(~cntpct);
}

void delay_timer_init_args(uint32_t mult, uint32_t div)
{
	ops.get_timer_value	= timer_get_value,
	ops.clk_mult		= mult;
	ops.clk_div		= div;

	timer_init(&ops);

	VERBOSE("Generic delay timer configured with mult=%u and div=%u\n",
		mult, div);
}

/*
 * Initialise the nxp on-chip free rolling usec counter as the delay
 * timer.
 */
void delay_timer_init(void)
{
	/* Value in ticks */
	unsigned int mult = MHZ_TICKS_PER_SEC;

	unsigned int div;

	unsigned int counter_base_frequency = plat_get_syscnt_freq2();

	/* Rounding off the Counter Frequency to MHZ_TICKS_PER_SEC */
	if (counter_base_frequency > MHZ_TICKS_PER_SEC)
		counter_base_frequency = (counter_base_frequency
					/ MHZ_TICKS_PER_SEC)
					* MHZ_TICKS_PER_SEC;
	else
		counter_base_frequency = (counter_base_frequency
					/ KHZ_TICKS_PER_SEC)
					* KHZ_TICKS_PER_SEC;

	/* Value in ticks per second (Hz) */
	div = counter_base_frequency;

	/* Reduce multiplier and divider by dividing them repeatedly by 10 */
	while ((mult % 10 == 0) && (div % 10 == 0)) {
		mult /= 10;
		div /= 10;
	}

	/* Enable and initialize the System level generic timer */
	mmio_write_32(NXP_TIMER_ADDR + CNTCR_OFF,
			CNTCR_FCREQ(0) | CNTCR_EN);

	delay_timer_init_args(mult, div);
}
