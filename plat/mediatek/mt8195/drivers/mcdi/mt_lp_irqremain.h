/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_LP_IRQREMAIN_H
#define MT_LP_IRQREMAIN_H

extern int mt_lp_irqremain_submit(void);
extern int mt_lp_irqremain_aquire(void);
extern int mt_lp_irqremain_release(void);
extern void mt_lp_irqremain_init(void);
#endif /* MT_LP_IRQREMAIN_H */
