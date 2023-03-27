/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPE_H
#define SPE_H

#include <stdbool.h>

#if ENABLE_SPE_FOR_NS
void spe_enable(bool el2_unused);
void spe_disable(void);
#else
static inline void spe_enable(bool el2_unused)
{
}
static inline void spe_disable(void)
{
}
#endif

#endif /* SPE_H */
