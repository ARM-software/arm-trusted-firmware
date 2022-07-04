/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 */

#include <stdint.h>

#include <plat/common/platform.h>
#include <platform_def.h>

/*******************************************************************************
 * Check passed region is within Non-Secure region of DRAM
 ******************************************************************************/
int plat_drtm_validate_ns_region(uintptr_t region_start,
				 size_t region_size)
{
	uintptr_t region_end = region_start + region_size - 1;

	if (region_start >= region_end) {
		return -1;
	} else if ((region_start >= ARM_NS_DRAM1_BASE) &&
		   (region_start < (ARM_NS_DRAM1_BASE + ARM_NS_DRAM1_SIZE)) &&
		   (region_end >= ARM_NS_DRAM1_BASE) &&
		   (region_end < (ARM_NS_DRAM1_BASE + ARM_NS_DRAM1_SIZE))) {
		return 0;
	} else if ((region_start >= ARM_DRAM2_BASE) &&
		   (region_start < (ARM_DRAM2_BASE + ARM_DRAM2_SIZE)) &&
		   (region_end >= ARM_DRAM2_BASE) &&
		   (region_end < (ARM_DRAM2_BASE + ARM_DRAM2_SIZE))) {
		return 0;
	}

	return -1;
}
