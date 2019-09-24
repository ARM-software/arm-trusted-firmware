/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Linaro Limited
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DELAY_TIMER_H
#define DELAY_TIMER_H

#include <stdbool.h>
#include <stdint.h>

#include <arch_helpers.h>

/********************************************************************
 * A simple timer driver providing synchronous delay functionality.
 * The driver must be initialized with a structure that provides a
 * function pointer to return the timer value and a clock
 * multiplier/divider. The ratio of the multiplier and the divider is
 * the clock period in microseconds.
 ********************************************************************/

typedef struct timer_ops {
	uint32_t (*get_timer_value)(void);
	uint32_t clk_mult;
	uint32_t clk_div;
} timer_ops_t;

static inline uint64_t timeout_cnt_us2cnt(uint32_t us)
{
	return ((uint64_t)us * (uint64_t)read_cntfrq_el0()) / 1000000ULL;
}

static inline uint64_t timeout_init_us(uint32_t us)
{
	uint64_t cnt = timeout_cnt_us2cnt(us);

	cnt += read_cntfrq_el0();

	return cnt;
}

static inline bool timeout_elapsed(uint64_t expire_cnt)
{
	return read_cntpct_el0() > expire_cnt;
}

void mdelay(uint32_t msec);
void udelay(uint32_t usec);
void timer_init(const timer_ops_t *ops_ptr);

#endif /* DELAY_TIMER_H */
