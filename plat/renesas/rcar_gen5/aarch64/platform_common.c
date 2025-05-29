/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <plat_helpers.h>
#include <platform_def.h>
#include "rcar_def.h"
#include "rcar_private.h"
#include "rcar_version.h"

const uint8_t version_of_renesas[VERSION_OF_RENESAS_MAXLEN]
		__section(".ro") = VERSION_OF_RENESAS;

#define RCAR_DCACHE		MT_MEMORY

#define MAP_SHARED_RAM_2						\
	MAP_REGION_FLAT(PARAMS_BASE, PARAMS_SIZE,			\
			MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_SHARED_RAM							\
	MAP_REGION_FLAT(RCAR_SHARED_MEM_BASE, RCAR_SHARED_MEM_SIZE,	\
			MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_DEVICE_RCAR1						\
	MAP_REGION_FLAT(DEVICE_RCAR_BASE1, DEVICE_RCAR_SIZE1,		\
			MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE_RCAR2						\
	MAP_REGION_FLAT(DEVICE_RCAR_BASE2, DEVICE_RCAR_SIZE2,		\
			MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE_RCAR3						\
	MAP_REGION_FLAT(DEVICE_RCAR_BASE3, DEVICE_RCAR_SIZE3,		\
			MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_ATFW_CRASH							\
	MAP_REGION_FLAT(RCAR_BL31_CRASH_BASE, RCAR_BL31_CRASH_SIZE,	\
			MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_SRAM							\
	MAP_REGION_FLAT(DEVICE_SRAM_BASE, DEVICE_SRAM_SIZE,		\
			MT_MEMORY | MT_RO | MT_SECURE)

#define MAP_SRAM_DATA_STACK						\
	MAP_REGION_FLAT(DEVICE_SRAM_DATA_BASE,				\
			DEVICE_SRAM_DATA_SIZE + DEVICE_SRAM_STACK_SIZE,	\
			MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_SCMI_CHANNEL						\
	MAP_REGION_FLAT(RCAR_SCMI_CHANNEL_MMU_BASE,			\
			RCAR_SCMI_CHANNEL_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE)

static const mmap_region_t rcar_mmap[] = {
	MAP_DEVICE_RCAR1,
	MAP_SHARED_RAM,
	MAP_SHARED_RAM_2,
	MAP_ATFW_CRASH,
	MAP_DEVICE_RCAR2,
	MAP_SRAM,
	MAP_SRAM_DATA_STACK,
	MAP_SCMI_CHANNEL,
	MAP_DEVICE_RCAR3,
	{0}
};

CASSERT((ARRAY_SIZE(rcar_mmap) + RCAR_BL_REGIONS)
	<= MAX_MMAP_REGIONS, assert_max_mmap_regions);

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
	if (freq == 0U) {
		panic();
	}

	return freq;
}

unsigned int plat_arm_calc_core_pos(u_register_t mpidr)
{
	return plat_renesas_calc_core_pos(mpidr);
}
