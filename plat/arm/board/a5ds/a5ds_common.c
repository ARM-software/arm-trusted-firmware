/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>

#define MAP_PERIPHBASE	MAP_REGION_FLAT(PERIPHBASE,\
					PERIPH_SIZE,\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_A5_PERIPHERALS	MAP_REGION_FLAT(A5_PERIPHERALS_BASE,\
					A5_PERIPHERALS_SIZE,\
					MT_DEVICE | MT_RW | MT_SECURE)

#ifdef IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MAP_PERIPHBASE,
	MAP_A5_PERIPHERALS,
	MAP_BOOT_RW,
	{0}
};
#endif
#ifdef IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MAP_PERIPHBASE,
	MAP_A5_PERIPHERALS,
	MAP_BOOT_RW,
	ARM_MAP_NS_DRAM1,
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	MAP_PERIPHBASE,
	MAP_A5_PERIPHERALS,
	{0}
};
#endif

ARM_CASSERT_MMAP

unsigned int plat_get_syscnt_freq2(void)
{
	return A5DS_TIMER_BASE_FREQUENCY;
}
