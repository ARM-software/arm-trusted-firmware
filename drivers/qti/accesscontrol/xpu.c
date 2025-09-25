/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>

#include <drivers/qti/accesscontrol/xpu.h>
#include <lib/mmio.h>

struct RGPartitionRangeType {
	uint32_t rg_num;
	uintptr_t start_addr;
	uintptr_t end_addr;
};

struct xpuInstanceType {
	uintptr_t xpu_base_addr;
	uint64_t owner_arr_size;
	void *rg_owner;
	uint64_t part_range_arr_size;
	struct RGPartitionRangeType *partition_range;
	int xpu_id;
	uint32_t flag;
};

#ifdef QTISECLIB_PATH
extern struct xpuInstanceType msm_xpu_cfg[];
extern uint32_t msm_xpu_cfg_count;
#else
static struct xpuInstanceType msm_xpu_cfg[0];
static uint32_t msm_xpu_cfg_count;
#endif

void qti_msm_xpu_bypass(void)
{
	for (int i = 0; i < msm_xpu_cfg_count; i++) {
		struct xpuInstanceType *xpu = &msm_xpu_cfg[i];

		for (int j = 0; j < xpu->part_range_arr_size; j++) {
			xpu->partition_range[j].start_addr = 0xffffffff;
			xpu->partition_range[j].end_addr = 0xffffffff;
		}
	}
}
