/*
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPE_H
#define SPE_H

#include <stdbool.h>
#include <context.h>

#if ENABLE_SPE_FOR_NS
void spe_enable(cpu_context_t *ctx);
void spe_disable(cpu_context_t *ctx);
void spe_init_el2_unused(void);
void spe_stop(void);
#else
static inline void spe_enable(cpu_context_t *ctx)
{
}
static inline void spe_disable(cpu_context_t *ctx)
{
}
static inline void spe_init_el2_unused(void)
{
}
static inline void spe_stop(void)
{
}
#endif /* ENABLE_SPE_FOR_NS */

#endif /* SPE_H */
