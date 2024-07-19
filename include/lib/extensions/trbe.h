/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRBE_H
#define TRBE_H

#include <context.h>

#if ENABLE_TRBE_FOR_NS
void trbe_init_el3(void);
void trbe_init_el2_unused(void);
void trbe_disable(void);
#else
static inline void trbe_disable(void)
{
}
static inline void trbe_init_el3(void)
{
}
static inline void trbe_init_el2_unused(void)
{
}
#endif /* ENABLE_TRBE_FOR_NS */

#endif /* TRBE_H */
