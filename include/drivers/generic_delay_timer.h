/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GENERIC_DELAY_TIMER_H__
#define __GENERIC_DELAY_TIMER_H__

#include <stdint.h>

void generic_delay_timer_init_args(uint32_t mult, uint32_t div);

void generic_delay_timer_init(void);

#endif /* __GENERIC_DELAY_TIMER_H__ */
