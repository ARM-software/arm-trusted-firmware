/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPA2_H
#define CPA2_H

#if ENABLE_FEAT_CPA2
void cpa2_enable_el3(void);
#else
static inline void cpa2_enable_el3(void)
{
}
#endif

#endif /* CPA2_H */
