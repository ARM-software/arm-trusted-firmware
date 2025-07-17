/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPE_H
#define SPE_H

#include <stdbool.h>
#include <context.h>

#if ENABLE_SPE_FOR_NS
void spe_enable_ns(cpu_context_t *ctx);
void spe_disable_secure(cpu_context_t *ctx);
void spe_disable_realm(cpu_context_t *ctx);
void spe_init_el2_unused(void);
#else
static inline void spe_enable_ns(cpu_context_t *ctx)
{
}
static inline void spe_disable_secure(cpu_context_t *ctx)
{
}
static inline void spe_disable_realm(cpu_context_t *ctx)
{
}
static inline void spe_init_el2_unused(void)
{
}
#endif /* ENABLE_SPE_FOR_NS */

#endif /* SPE_H */
