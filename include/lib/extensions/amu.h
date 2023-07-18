/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AMU_H
#define AMU_H

#include <stdbool.h>
#include <stdint.h>

#include <context.h>

#include <platform_def.h>

#if ENABLE_FEAT_AMU
#if __aarch64__
void amu_enable(cpu_context_t *ctx);
void amu_init_el3(void);
void amu_init_el2_unused(void);
void amu_enable_per_world(per_world_context_t *per_world_ctx);
#else
void amu_enable(bool el2_unused);
#endif /* __aarch64__ */

#else
#if __aarch64__
void amu_enable(cpu_context_t *ctx)
{
}
void amu_init_el3(void)
{
}
void amu_init_el2_unused(void)
{
}
void amu_enable_per_world(per_world_context_t *per_world_ctx)
{
}
#else
static inline void amu_enable(bool el2_unused)
{
}
#endif /*__aarch64__ */
#endif /* ENABLE_FEAT_AMU */

#if ENABLE_AMU_AUXILIARY_COUNTERS
/*
 * AMU data for a single core.
 */
struct amu_core {
	uint16_t enable; /* Mask of auxiliary counters to enable */
};

/*
 * Topological platform data specific to the AMU.
 */
struct amu_topology {
	struct amu_core cores[PLATFORM_CORE_COUNT]; /* Per-core data */
};

#if !ENABLE_AMU_FCONF
/*
 * Retrieve the platform's AMU topology. A `NULL` return value is treated as a
 * non-fatal error, in which case no auxiliary counters will be enabled.
 */
const struct amu_topology *plat_amu_topology(void);
#endif /* ENABLE_AMU_FCONF */
#endif /* ENABLE_AMU_AUXILIARY_COUNTERS */

#endif /* AMU_H */
