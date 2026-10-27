/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMMU_H
#define SMMU_H

#include <stddef.h>
#include <stdint.h>

#ifdef QTI_SMMU_ENABLED
struct smmu_cfg_s {
	uintptr_t addr;
	uint32_t value;
};

void qti_smmu_init(void);
void qti_smmu_clocked_init(void);
extern struct smmu_cfg_s qti_smmu_cfg[];
extern const size_t qti_smmu_cfg_count;
extern const struct smmu_cfg_s qti_smmu_gpu_cfg[];
extern const size_t qti_smmu_gpu_cfg_count;
#else
static inline void qti_smmu_init(void) {}
static inline void qti_smmu_clocked_init(void) {}
#endif

#endif
