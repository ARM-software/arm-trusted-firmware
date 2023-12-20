/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/arm/css/sds.h>
#include <drivers/arm/sbsa.h>
#include <lib/utils_def.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <platform_def.h>

/*
 * Table of regions for different BL stages to map using the MMU.
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	NRD_CSS_SHARED_RAM_MMAP(0),
	NRD_ROS_FLASH0_RO_MMAP,
	NRD_CSS_PERIPH_MMAP(0),
	NRD_ROS_PLATFORM_PERIPH_MMAP,
	NRD_ROS_SYSTEM_PERIPH_MMAP,
	{0}
};
#endif /* IMAGE_BL3 */

#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	NRD_CSS_SHARED_RAM_MMAP(0),
	NRD_ROS_FLASH0_RO_MMAP,
#ifdef PLAT_ARM_MEM_PROT_ADDR
	NRD_ROS_V2M_MEM_PROTECT_MMAP,
#endif
	NRD_CSS_PERIPH_MMAP(0),
	NRD_ROS_PLATFORM_PERIPH_MMAP,
	NRD_ROS_SYSTEM_PERIPH_MMAP,
	NRD_CSS_NS_DRAM1_MMAP,
#if TRUSTED_BOARD_BOOT && !RESET_TO_BL2
	NRD_CSS_BL1_RW_MMAP,
#endif
	NRD_CSS_GPT_L1_DRAM_MMAP,
	NRD_CSS_RMM_REGION_MMAP,
	{0}
};
#endif /* IMAGE_BL2 */

#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	NRD_CSS_SHARED_RAM_MMAP(0),
#ifdef PLAT_ARM_MEM_PROT_ADDR
	NRD_ROS_V2M_MEM_PROTECT_MMAP,
#endif
	NRD_CSS_PERIPH_MMAP(0),
	NRD_ROS_PLATFORM_PERIPH_MMAP,
	NRD_ROS_SYSTEM_PERIPH_MMAP,
	NRD_CSS_GPT_L1_DRAM_MMAP,
	NRD_CSS_EL3_RMM_SHARED_MEM_MMAP,
	{0}
};
#endif /* IMAGE_BL31 */

ARM_CASSERT_MMAP

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
	sbsa_wdog_start(NRD_CSS_AP_SECURE_WDOG_BASE,
			NRD_CSS_AP_SECURE_WDOG_TIMEOUT);
}

void plat_arm_secure_wdt_stop(void)
{
	sbsa_wdog_stop(NRD_CSS_AP_SECURE_WDOG_BASE);
}

static sds_region_desc_t nrd_sds_regions[] = {
	{ .base = PLAT_ARM_SDS_MEM_BASE },
};

sds_region_desc_t *plat_sds_get_regions(unsigned int *region_count)
{
	*region_count = ARRAY_SIZE(nrd_sds_regions);

	return nrd_sds_regions;
}
