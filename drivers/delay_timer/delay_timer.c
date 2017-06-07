/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <delay_timer.h>
#include <platform_def.h>

/***********************************************************
 * The delay timer implementation
 ***********************************************************/
static const timer_ops_t *ops;

/***********************************************************
 * Delay for the given number of microseconds. The driver must
 * be initialized before calling this function.
 ***********************************************************/
void udelay(uint32_t usec)
{
	assert(ops != NULL &&
		(ops->clk_mult != 0) &&
		(ops->clk_div != 0) &&
		(ops->get_timer_value != NULL));

	uint32_t start, delta, total_delta;

	assert(usec < UINT32_MAX / ops->clk_div);

	start = ops->get_timer_value();

	total_delta = (usec * ops->clk_div) / ops->clk_mult;

	do {
		/*
		 * If the timer value wraps around, the subtraction will
		 * overflow and it will still give the correct result.
		 */
		delta = start - ops->get_timer_value(); /* Decreasing counter */

	} while (delta < total_delta);
}

/***********************************************************
 * Delay for the given number of milliseconds. The driver must
 * be initialized before calling this function.
 ***********************************************************/
void mdelay(uint32_t msec)
{
	udelay(msec*1000);
}

/***********************************************************
 * Initialize the timer. The fields in the provided timer
 * ops pointer must be valid.
 ***********************************************************/
void timer_init(const timer_ops_t *ops_ptr)
{
	assert(ops_ptr != NULL  &&
		(ops_ptr->clk_mult != 0) &&
		(ops_ptr->clk_div != 0) &&
		(ops_ptr->get_timer_value != NULL));

	ops = ops_ptr;
}
