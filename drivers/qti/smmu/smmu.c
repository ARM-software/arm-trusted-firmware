/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <drivers/qti/chipinfo/chipinfo.h>
#include <drivers/qti/smmu/smmu.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

void qti_smmu_init(void)
{
	for (int i = 0; i < qti_smmu_cfg_count; i++) {
		mmio_write_32(qti_smmu_cfg[i].addr, qti_smmu_cfg[i].value);
	}
}

/*
 * The GPU SMMU sits in GPU CX, which the clock back-end powers and clocks,
 * with register retention on, only around clocked_boot_init(). A disabled
 * GPU part has no GPU SMMU state to set.
 */
void qti_smmu_clocked_init(void)
{
	if (chipinfo_is_part_disabled(CHIPINFO_PART_GPU, 0U)) {
		return;
	}

	for (size_t i = 0U; i < qti_smmu_gpu_cfg_count; i++) {
		mmio_write_32(qti_smmu_gpu_cfg[i].addr,
			      qti_smmu_gpu_cfg[i].value);
	}
}
