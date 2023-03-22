/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MPAM_H
#define MPAM_H

#include <stdbool.h>

#if ENABLE_MPAM_FOR_LOWER_ELS
void mpam_enable(bool el2_unused);
#else
static inline void mpam_enable(bool el2_unused)
{
}
#endif

#endif /* MPAM_H */
