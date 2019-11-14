/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <drivers/delay_timer.h>
#include <lib/utils_def.h>

/***********************************************************
 * The delay timer implementation
 ***********************************************************/
static const timer_ops_t *timer_ops;

/***********************************************************
 * Delay for the given number of microseconds. The driver must
 * be initialized before calling this function.
 ***********************************************************/
void udelay(uint32_t usec)
{
	assert((timer_ops != NULL) &&
		(timer_ops->clk_mult != 0U) &&
		(timer_ops->clk_div != 0U) &&
		(timer_ops->get_timer_value != NULL));

	uint32_t start, delta;
	uint64_t total_delta;

	assert(usec < (UINT64_MAX / timer_ops->clk_div));

	start = timer_ops->get_timer_value();

	/* Add an extra tick to avoid delaying less than requested. */
	total_delta =
		div_round_up((uint64_t)usec * timer_ops->clk_div,
						timer_ops->clk_mult) + 1U;
	/*
	 * Precaution for the total_delta ~ UINT32_MAX and the fact that we
	 * cannot catch every tick of the timer.
	 * For example 100MHz timer over 25MHz APB will miss at least 4 ticks.
	 * 1000U is an arbitrary big number which is believed to be sufficient.
	 */
	assert(total_delta < (UINT32_MAX - 1000U));

	do {
		/*
		 * If the timer value wraps around, the subtraction will
		 * overflow and it will still give the correct result.
		 * delta is decreasing counter
		 */
		delta = start - timer_ops->get_timer_value();

	} while (delta < total_delta);
}

/***********************************************************
 * Delay for the given number of milliseconds. The driver must
 * be initialized before calling this function.
 ***********************************************************/
void mdelay(uint32_t msec)
{
	assert((msec * 1000UL) < UINT32_MAX);
	udelay(msec * 1000U);
}

/***********************************************************
 * Initialize the timer. The fields in the provided timer
 * ops pointer must be valid.
 ***********************************************************/
void timer_init(const timer_ops_t *ops_ptr)
{
	assert((ops_ptr != NULL)  &&
		(ops_ptr->clk_mult != 0U) &&
		(ops_ptr->clk_div != 0U) &&
		(ops_ptr->get_timer_value != NULL));

	timer_ops = ops_ptr;
}
