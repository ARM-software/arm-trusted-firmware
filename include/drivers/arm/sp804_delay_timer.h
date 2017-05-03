/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SP804_DELAY_TIMER_H__
#define __SP804_DELAY_TIMER_H__

#include <delay_timer.h>
#include <stdint.h>


uint32_t sp804_get_timer_value(void);

void sp804_timer_ops_init(uintptr_t base_addr, const timer_ops_t *ops);

#define sp804_timer_init(base_addr, clk_mult, clk_div)			\
	do {								\
		static const timer_ops_t sp804_timer_ops = {		\
			sp804_get_timer_value,				\
			(clk_mult),					\
			(clk_div)					\
		};							\
		sp804_timer_ops_init((base_addr), &sp804_timer_ops);	\
	} while (0)

#endif /* __SP804_DELAY_TIMER_H__ */
