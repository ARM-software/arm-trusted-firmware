/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FPMR_H
#define FPMR_H

#include <context.h>

#if ENABLE_FEAT_FPMR
void fpmr_enable_per_world(per_world_context_t *per_world_ctx);
#else
static inline void fpmr_enable_per_world(per_world_context_t *per_world_ctx)
{
}
#endif /* ENABLE_FEAT_FPMR */

#endif /* FPMR_H */
