/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_CPUXGPT_H
#define MT_CPUXGPT_H

/* REG */
#define INDEX_CTL_REG       0x000
#define INDEX_STA_REG       0x004
#define INDEX_CNT_L_INIT    0x008
#define INDEX_CNT_H_INIT    0x00C

/* CTL_REG SET */
#define EN_CPUXGPT          0x01
#define EN_AHLT_DEBUG       0x02
#define CLK_DIV1            (0x1 << 8)
#define CLK_DIV2            (0x2 << 8)
#define CLK_DIV4            (0x4 << 8)
#define CLK_DIV_MASK        (~(0x7<<8))

void generic_timer_backup(void);
void sched_clock_init(uint64_t normal_base, uint64_t atf_base);
uint64_t sched_clock(void);

#endif /* MT_CPUXGPT_H */
