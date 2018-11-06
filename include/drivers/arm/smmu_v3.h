/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_V3_H
#define SMMU_V3_H

#include <stdint.h>
#include <utils_def.h>

/* SMMUv3 register offsets from device base */
#define SMMU_S_IDR1	U(0x8004)
#define SMMU_S_INIT	U(0x803c)

/* SMMU_S_IDR1 register fields */
#define SMMU_S_IDR1_SECURE_IMPL_SHIFT	31
#define SMMU_S_IDR1_SECURE_IMPL_MASK	U(0x1)

/* SMMU_S_INIT register fields */
#define SMMU_S_INIT_INV_ALL_MASK	U(0x1)


int smmuv3_init(uintptr_t smmu_base);

#endif /* SMMU_V3_H */
