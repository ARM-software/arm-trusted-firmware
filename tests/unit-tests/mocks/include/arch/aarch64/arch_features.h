/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_FEATURES_H
#define ARCH_FEATURES_H

#include <stdbool.h>

__attribute__((always_inline))
static inline bool is_feat_mte2_supported(void)
{
    return false;
}

static inline bool is_feat_rme_gpc2_present(void)
{
    return false;
}

static inline bool is_feat_rme_gdi_supported(void)
{
    return false;
}

#endif /* ARCH_FEATURES_H */