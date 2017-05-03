/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MT_CPUXGPT_H__
#define __MT_CPUXGPT_H__

/* REG */
#define INDEX_CNT_L_INIT    0x008
#define INDEX_CNT_H_INIT    0x00C

void generic_timer_backup(void);

#endif /* __MT_CPUXGPT_H__ */
