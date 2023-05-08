/*
 * Copyright (c) 2020-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/arm/css/sds.h>
#include <drivers/arm/sbsa.h>
#include <lib/utils_def.h>
#include <plat/arm/common/plat_arm.h>

#include "morello_def.h"

/*
 * Table of regions to map using the MMU.
 * Replace or extend the below regions as required
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MORELLO_MAP_DEVICE,
	MORELLO_MAP_NS_SRAM,
	ARM_MAP_DRAM1,
	ARM_MAP_DRAM2,
	{0}
};
#endif

#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MORELLO_MAP_DEVICE,
	MORELLO_MAP_NS_SRAM,
	{0}
};
#endif

#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MORELLO_MAP_DEVICE,
	MORELLO_MAP_NS_SRAM,
	ARM_MAP_DRAM1,
	ARM_MAP_DRAM2,
#if TRUSTED_BOARD_BOOT && !RESET_TO_BL2
	ARM_MAP_BL1_RW,
#endif
	{0}
};
#endif

#if TRUSTED_BOARD_BOOT
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);

	return arm_get_mbedtls_heap(heap_addr, heap_size);
}
#endif

void plat_arm_secure_wdt_start(void)
{
	sbsa_wdog_start(SBSA_SECURE_WDOG_BASE, SBSA_SECURE_WDOG_TIMEOUT);
}

void plat_arm_secure_wdt_stop(void)
{
	sbsa_wdog_stop(SBSA_SECURE_WDOG_BASE);
}

#if CSS_USE_SCMI_SDS_DRIVER
static sds_region_desc_t morello_sds_regions[] = {
	{ .base = PLAT_ARM_SDS_MEM_BASE },
};

sds_region_desc_t *plat_sds_get_regions(unsigned int *region_count)
{
	*region_count = ARRAY_SIZE(morello_sds_regions);

	return morello_sds_regions;
}
#endif /* CSS_USE_SCMI_SDS_DRIVER */
