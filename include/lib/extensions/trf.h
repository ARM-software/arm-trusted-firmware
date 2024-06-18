/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRF_H
#define TRF_H

#include <context.h>

#if ENABLE_TRF_FOR_NS

#if __aarch64__
void trf_enable(cpu_context_t *ctx);
void trf_init_el2_unused(void);
#else /* !__aarch64 */
void trf_init_el3(void);
#endif /* __aarch64__ */

#else /* ENABLE_TRF_FOR_NS=0 */

#if __aarch64__
static inline void trf_enable(cpu_context_t *ctx)
{
}
static inline void trf_init_el2_unused(void)
{
}
#else /* !__aarch64 */
static inline void trf_init_el3(void)
{
}
#endif /* __aarch64__*/

#endif /* ENABLE_TRF_FOR_NS */

#endif /* TRF_H */
