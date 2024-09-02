/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TCR2_H
#define TCR2_H

#include <context.h>

#if ENABLE_FEAT_TCR2
void tcr2_enable(cpu_context_t *ctx);
void tcr2_disable(cpu_context_t *ctx);
#else
static inline void tcr2_enable(cpu_context_t *ctx)
{
}
static inline void tcr2_disable(cpu_context_t *ctx)
{
}
#endif /* ENABLE_FEAT_TCR2 */

#endif /* TCR2_H */
