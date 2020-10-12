/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_TIMER_H
#define MT_TIMER_H


#define SYSTIMER_BASE       (0x10017000)
#define CNTCR_REG           (SYSTIMER_BASE + 0x0)
#define CNTSR_REG           (SYSTIMER_BASE + 0x4)

#define COMP_15_EN          (1 << 10)
#define COMP_20_EN          (1 << 11)

void mt_systimer_init(void);

#endif /* MT_TIMER_H */
