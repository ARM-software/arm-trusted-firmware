/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SME_H
#define SME_H

#include <stdbool.h>
#include <context.h>

/*
 * Maximum value of LEN field in SMCR_ELx. This is different than the maximum
 * supported value which is platform dependent. In the first version of SME the
 * LEN field is limited to 4 bits but will be expanded in future iterations.
 * To support different versions, the code that discovers the supported vector
 * lengths will write the max value into SMCR_ELx then read it back to see how
 * many bits are implemented.
 */
#define SME_SMCR_LEN_MAX	U(0x1FF)

#if ENABLE_SME_FOR_NS
void sme_init_el3(void);
void sme_init_el2_unused(void);
void sme_enable(cpu_context_t *context);
void sme_disable(cpu_context_t *context);
void sme_enable_per_world(per_world_context_t *per_world_ctx);
void sme_disable_per_world(per_world_context_t *per_world_ctx);
#else
static inline void sme_init_el3(void)
{
}
static inline void sme_init_el2_unused(void)
{
}
static inline void sme_enable(cpu_context_t *context)
{
}
static inline void sme_disable(cpu_context_t *context)
{
}
static inline void sme_enable_per_world(per_world_context_t *per_world_ctx)
{
}
static inline void sme_disable_per_world(per_world_context_t *per_world_ctx)
{
}
#endif /* ENABLE_SME_FOR_NS */

#endif /* SME_H */
