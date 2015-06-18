/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
	assert(ops != 0 &&
		(ops->clk_mult != 0) &&
		(ops->clk_div != 0) &&
		(ops->get_timer_value != 0));

	uint32_t start, cnt, delta, delta_us;

	/* counter is decreasing */
	start = ops->get_timer_value();
	do {
		cnt = ops->get_timer_value();
		if (cnt > start) {
			delta = UINT32_MAX - cnt;
			delta += start;
		} else
			delta = start - cnt;
		delta_us = (delta * ops->clk_mult) / ops->clk_div;
	} while (delta_us < usec);
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
	assert(ops_ptr != 0  &&
		(ops_ptr->clk_mult != 0) &&
		(ops_ptr->clk_div != 0) &&
		(ops_ptr->get_timer_value != 0));

	ops = ops_ptr;
}
