/*
 * Copyright (c) 2013-2025, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2015-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <plat_helpers.h>
#include <platform_def.h>

#include "rcar_def.h"
#include "rcar_private.h"
#include "rcar_version.h"

const uint8_t version_of_renesas[VERSION_OF_RENESAS_MAXLEN]
		__section("ro") = VERSION_OF_RENESAS;

#define RCAR_DCACHE		MT_MEMORY

#define MAP_SHARED_RAM		MAP_REGION_FLAT(RCAR_SHARED_MEM_BASE,	\
						RCAR_SHARED_MEM_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_DEVICE_RCAR		MAP_REGION_FLAT(DEVICE_RCAR_BASE,	\
						DEVICE_RCAR_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE_RCAR2	MAP_REGION_FLAT(DEVICE_RCAR_BASE2,	\
						DEVICE_RCAR_SIZE2,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_ATFW_CRASH		MAP_REGION_FLAT(RCAR_BL31_CRASH_BASE,	\
						RCAR_BL31_CRASH_SIZE,	\
						MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_SRAM		MAP_REGION_FLAT(DEVICE_SRAM_BASE,	\
						DEVICE_SRAM_SIZE,	\
						MT_MEMORY | MT_RO | MT_SECURE)

#define MAP_SRAM_DATA_STACK	MAP_REGION_FLAT(DEVICE_SRAM_DATA_BASE,	\
						(DEVICE_SRAM_DATA_SIZE + \
						 DEVICE_SRAM_STACK_SIZE), \
						MT_MEMORY | MT_RW | MT_SECURE)

static const mmap_region_t rcar_mmap[] = {
	MAP_SHARED_RAM,	  /* 0x46422000 - 0x46422FFF  Shared ram area       */
	MAP_ATFW_CRASH,	  /* 0x4643F000 - 0x4643FFFF  Stack for Crash Log   */
	MAP_DEVICE_RCAR,  /* 0xE6000000 - 0xE62FFFFF  SoC registers area    */
	MAP_SRAM,         /* 0xE6342000 - 0xE6343FFF  System RAM code area  */
	MAP_SRAM_DATA_STACK, /* 0xE6344000 - 0xE6344FFF  System RAM data & stack area */
	MAP_DEVICE_RCAR2, /* 0xE6370000 - 0xFFFFFFFF  SoC registers area 2  */
	{ 0 }
};

CASSERT((ARRAY_SIZE(rcar_mmap) + RCAR_BL_REGIONS) <= MAX_MMAP_REGIONS,
	assert_max_mmap_regions);

/*
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 */
void rcar_configure_mmu_el3(uintptr_t total_base,
			    size_t total_size,
			    uintptr_t ro_start,
			    uintptr_t ro_limit)
{
	mmap_add_region(total_base, total_base, total_size,
			RCAR_DCACHE | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_start, ro_limit - ro_start,
			RCAR_DCACHE | MT_RO | MT_SECURE);
	mmap_add(rcar_mmap);

	init_xlat_tables();
	enable_mmu_el3(0);
}

unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int freq;

	freq = mmio_read_32(ARM_SYS_CNTCTL_BASE + CNTFID_OFF);
	if (freq == 0)
		panic();

	return freq;
}

unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
{
	return plat_renesas_calc_core_pos(mpidr);
}
