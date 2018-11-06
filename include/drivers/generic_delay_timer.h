/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GENERIC_DELAY_TIMER_H
#define GENERIC_DELAY_TIMER_H

#include <stdint.h>

void generic_delay_timer_init_args(uint32_t mult, uint32_t div);

void generic_delay_timer_init(void);

#endif /* GENERIC_DELAY_TIMER_H */
