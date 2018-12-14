/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
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

	uint32_t start, delta, total_delta;

	assert(usec < (UINT32_MAX / timer_ops->clk_div));

	start = timer_ops->get_timer_value();

	/* Add an extra tick to avoid delaying less than requested. */
	total_delta =
		div_round_up(usec * timer_ops->clk_div,
						timer_ops->clk_mult) + 1U;

	do {
		/*
		 * If the timer value wraps around, the subtraction will
		 * overflow and it will still give the correct result.
		 */
		delta = start - timer_ops->get_timer_value(); /* Decreasing counter */

	} while (delta < total_delta);
}

/***********************************************************
 * Delay for the given number of milliseconds. The driver must
 * be initialized before calling this function.
 ***********************************************************/
void mdelay(uint32_t msec)
{
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
