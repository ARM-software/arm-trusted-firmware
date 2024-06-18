/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BRBE_H
#define BRBE_H

#include <context.h>

#if ENABLE_BRBE_FOR_NS
void brbe_enable(cpu_context_t *ctx);
#else
static inline void brbe_enable(cpu_context_t *ctx)
{
}
#endif /* ENABLE_BRBE_FOR_NS */

#endif /* BRBE_H */
