/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
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
	uint64_t (*timeout_init_us)(uint32_t usec);
	bool (*timeout_elapsed)(uint64_t cnt);
} timer_ops_t;

uint64_t timeout_init_us(uint32_t usec);
bool timeout_elapsed(uint64_t cnt);
void mdelay(uint32_t msec);
void udelay(uint32_t usec);
void timer_init(const timer_ops_t *ops_ptr);

#endif /* DELAY_TIMER_H */
