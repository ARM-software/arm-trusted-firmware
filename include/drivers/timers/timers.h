/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TIMERS_H__
#define __TIMERS_H__

/********************************************************************
 * A timer driver to handle multiple periodic or one-shot async
 * function call-backs.
 ********************************************************************/

typedef void (*hw_timer_isr_t)(void);

typedef struct timers_ops {
	int (*timer_register_irq)(hw_timer_isr_t isr);
	void (*timer_enable)(int enable);
	void (*timer_set_period)(uint64_t period);
	uint64_t (*timer_ms_to_ticks)(uint32_t time);
	uint64_t (*timer_get_remainig)(void);
} timers_ops_t;

typedef enum {
	TM_ONE_SHOT,
	TM_PERIODIC
} timer_kind_t;

typedef int (*timer_callback_t)(int hd);

typedef struct _timer_t {
	int is_created;
	int is_started;
	uint64_t period; /* in ticks */
	uint32_t fire_in;
	timer_kind_t type;
	timer_callback_t cb;
} timer_data;

void timers_init(const timers_ops_t *ops_ptr);

int timer_stop(int hd);
int timer_set_period(int hd, uint32_t period);
int timer_start(int hd);
int timer_create(timer_kind_t type, uint32_t period, timer_callback_t cb);

#endif

