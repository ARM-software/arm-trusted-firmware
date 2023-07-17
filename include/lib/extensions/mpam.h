/*
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPAM_H
#define MPAM_H

#include <stdbool.h>

#if ENABLE_MPAM_FOR_LOWER_ELS
void mpam_init_el3(void);
void mpam_init_el2_unused(void);
#else
static inline void mpam_init_el3(void)
{
}
static inline void mpam_init_el2_unused(void)
{
}
#endif /* ENABLE_MPAM_FOR_LOWER_ELS */

#endif /* MPAM_H */
