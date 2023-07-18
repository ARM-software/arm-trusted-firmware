/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SVE_H
#define SVE_H

#include <context.h>

#if (ENABLE_SME_FOR_NS || ENABLE_SVE_FOR_NS)
void sve_init_el2_unused(void);
void sve_enable_per_world(per_world_context_t *per_world_ctx);
void sve_disable_per_world(per_world_context_t *per_world_ctx);
#else
static inline void sve_init_el2_unused(void)
{
}
static inline void sve_enable_per_world(per_world_context_t *per_world_ctx)
{
}
static inline void sve_disable_per_world(per_world_context_t *per_world_ctx)
{
}
#endif /* ( ENABLE_SME_FOR_NS | ENABLE_SVE_FOR_NS ) */

#endif /* SVE_H */
