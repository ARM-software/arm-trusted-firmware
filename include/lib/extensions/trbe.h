/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRBE_H
#define TRBE_H

#include <context.h>

#if ENABLE_TRBE_FOR_NS
void trbe_disable(cpu_context_t *ctx);
void trbe_enable(cpu_context_t *ctx);
void trbe_init_el2_unused(void);
#else
static inline void trbe_disable(cpu_context_t *ctx)
{
}
static inline void trbe_enable(cpu_context_t *ctx)
{
}
static inline void trbe_init_el2_unused(void)
{
}
#endif /* ENABLE_TRBE_FOR_NS */

#endif /* TRBE_H */
