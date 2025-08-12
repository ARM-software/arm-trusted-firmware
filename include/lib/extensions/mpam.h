/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPAM_H
#define MPAM_H

#include <stdbool.h>

#include <context.h>

#if ENABLE_FEAT_MPAM
void mpam_enable_per_world(per_world_context_t *per_world_ctx);
void mpam_init_el2_unused(void);
void mpam_init_el3(void);
#else
static inline void mpam_enable_per_world(per_world_context_t *per_world_ctx)
{
}
static inline void mpam_init_el2_unused(void)
{
}
static inline void mpam_init_el3(void)
{
}
#endif /* ENABLE_FEAT_MPAM */

#endif /* MPAM_H */
