/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <plat/common/platform.h>

#include <arm_def.h>
#include <plat_arm.h>

/*
 * Table of regions to map using the MMU.
 * Replace or extend the below regions as required
 */

const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	N1SDP_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};

