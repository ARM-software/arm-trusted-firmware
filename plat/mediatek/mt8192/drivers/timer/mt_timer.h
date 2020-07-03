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
#define CNTSYS_L_REG        (SYSTIMER_BASE + 0x8)
#define CNTSYS_H_REG        (SYSTIMER_BASE + 0xc)

#define TIEO_EN             (1 << 3)
#define COMP_15_EN          (1 << 10)
#define COMP_20_EN          (1 << 11)
#define COMP_25_EN          (1 << 12)

#define COMP_FEATURE_MASK (COMP_15_EN | COMP_20_EN | COMP_25_EN | TIEO_EN)
#define COMP_15_MASK (COMP_15_EN)
#define COMP_20_MASK (COMP_20_EN | TIEO_EN)
#define COMP_25_MASK (COMP_20_EN | COMP_25_EN)


void sched_clock_init(uint64_t normal_base, uint64_t atf_base);
uint64_t sched_clock(void);

#endif /* MT_TIMER_H */
