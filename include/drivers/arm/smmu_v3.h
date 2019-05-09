/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_V3_H
#define SMMU_V3_H

#include <stdint.h>
#include <lib/utils_def.h>

/* SMMUv3 register offsets from device base */
#define SMMU_GBPA	U(0x0044)
#define SMMU_S_IDR1	U(0x8004)
#define SMMU_S_INIT	U(0x803c)
#define SMMU_S_GBPA	U(0x8044)

/* SMMU_GBPA register fields */
#define SMMU_GBPA_UPDATE		(1UL << 31)
#define SMMU_GBPA_ABORT			(1UL << 20)

/* SMMU_S_IDR1 register fields */
#define SMMU_S_IDR1_SECURE_IMPL		(1UL << 31)

/* SMMU_S_INIT register fields */
#define SMMU_S_INIT_INV_ALL		(1UL << 0)

/* SMMU_S_GBPA register fields */
#define SMMU_S_GBPA_UPDATE		(1UL << 31)
#define SMMU_S_GBPA_ABORT		(1UL << 20)

int smmuv3_init(uintptr_t smmu_base);
int smmuv3_security_init(uintptr_t smmu_base);

#endif /* SMMU_V3_H */
