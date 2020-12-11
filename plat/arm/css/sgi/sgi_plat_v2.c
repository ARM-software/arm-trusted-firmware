/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <drivers/arm/sbsa.h>

/*
 * Table of regions for different BL stages to map using the MMU.
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	SGI_MAP_FLASH0_RO,
	CSS_SGI_MAP_DEVICE,
	SOC_PLATFORM_PERIPH_MAP_DEVICE,
	SOC_SYSTEM_PERIPH_MAP_DEVICE,
	{0}
};
#endif

#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	SGI_MAP_FLASH0_RO,
#ifdef PLAT_ARM_MEM_PROT_ADDR
	ARM_V2M_MAP_MEM_PROTECT,
#endif
	CSS_SGI_MAP_DEVICE,
	SOC_MEMCNTRL_MAP_DEVICE,
	SOC_PLATFORM_PERIPH_MAP_DEVICE,
	SOC_SYSTEM_PERIPH_MAP_DEVICE,
	ARM_MAP_NS_DRAM1,
#if ARM_BL31_IN_DRAM
	ARM_MAP_BL31_SEC_DRAM,
#endif
#if TRUSTED_BOARD_BOOT && !BL2_AT_EL3
	ARM_MAP_BL1_RW,
#endif
	{0}
};
#endif

#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
#ifdef PLAT_ARM_MEM_PROT_ADDR
	ARM_V2M_MAP_MEM_PROTECT,
#endif
	CSS_SGI_MAP_DEVICE,
	SOC_PLATFORM_PERIPH_MAP_DEVICE,
	SOC_SYSTEM_PERIPH_MAP_DEVICE,
	{0}
};

#endif

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
	sbsa_wdog_start(SBSA_SECURE_WDOG_BASE, SBSA_SECURE_WDOG_TIMEOUT);
}

void plat_arm_secure_wdt_stop(void)
{
	sbsa_wdog_stop(SBSA_SECURE_WDOG_BASE);
}
