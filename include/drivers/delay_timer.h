/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DELAY_TIMER_H__
#define __DELAY_TIMER_H__

#include <stdint.h>

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

void mdelay(uint32_t msec);
void udelay(uint32_t usec);
void timer_init(const timer_ops_t *ops);


#endif /* __DELAY_TIMER_H__ */
