/*
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPAM_H
#define MPAM_H

#include <stdbool.h>

#include <context.h>

#if ENABLE_FEAT_MPAM
void mpam_enable(cpu_context_t *context);
void mpam_init_el2_unused(void);
#else
static inline void mpam_enable(cpu_context_t *context)
{
}
static inline void mpam_init_el2_unused(void)
{
}
#endif /* ENABLE_FEAT_MPAM */

#endif /* MPAM_H */
