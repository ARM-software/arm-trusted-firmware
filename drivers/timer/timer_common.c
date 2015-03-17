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

#include <debug.h>
#include <timer.h>
#include <platform_def.h>

/***********************************************************
 * The delay implementation for all BL stages
 ***********************************************************/
static const timer_ops_t *ops;

/***********************************************************
 * delay for the given number of microseconds
 ***********************************************************/
void udelay(uint32_t usec)
{
	uint32_t start, cnt, delta, delta_us;

	if (ops && ops->get_timer_value) {
		/* counter is decreasing */
		start = ops->get_timer_value();
		do {
			cnt = ops->get_timer_value();
			if (cnt > start) {
				delta = UINT32_MAX - cnt;
				delta += start;
			} else
				delta = start - cnt;
			delta_us = (delta * ops->clkmult) / ops->clkdiv;
		} while (delta_us < usec);
	} else
		ERROR("no get_timer_value() function\n");
}

/***********************************************************
 * delay for the given number of milliseconds
 ***********************************************************/
void mdelay(uint32_t msec)
{
	udelay(msec*1000);
}

/***********************************************************
 * delay for the given number of milliseconds
 ***********************************************************/
void timer_init(const timer_ops_t *ops_ptr)
{
	if (ops_ptr && (ops_ptr->clkmult != 0) && (ops_ptr->clkdiv != 0))
		ops = ops_ptr;
	else
		ERROR("timer_init(): invalid ops\n");
}
