/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SVE_H
#define SVE_H

#include <context.h>

#if (ENABLE_SME_FOR_NS || ENABLE_SVE_FOR_NS)
void sve_enable(cpu_context_t *context);
void sve_disable(cpu_context_t *context);
#else
static inline void sve_enable(cpu_context_t *context)
{
}
static inline void sve_disable(cpu_context_t *context)
{
}
#endif /* ( ENABLE_SME_FOR_NS | ENABLE_SVE_FOR_NS ) */

#endif /* SVE_H */
