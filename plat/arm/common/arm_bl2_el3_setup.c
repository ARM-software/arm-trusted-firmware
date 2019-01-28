/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/generic_delay_timer.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#pragma weak bl2_el3_early_platform_setup
#pragma weak bl2_el3_plat_arch_setup
#pragma weak bl2_el3_plat_prepare_exit

#define MAP_BL2_EL3_TOTAL	MAP_REGION_FLAT(				\
					bl2_el3_tzram_layout.total_base,	\
					bl2_el3_tzram_layout.total_size,	\
					MT_MEMORY | MT_RW | MT_SECURE)

static meminfo_t bl2_el3_tzram_layout;

/*
 * Perform arm specific early platform setup. At this moment we only initialize
 * the console and the memory layout.
 */
void arm_bl2_el3_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

	/*
	 * Allow BL2 to see the whole Trusted RAM. This is determined
	 * statically since we cannot rely on BL1 passing this information
	 * in the BL2_AT_EL3 case.
	 */
	bl2_el3_tzram_layout.total_base = ARM_BL_RAM_BASE;
	bl2_el3_tzram_layout.total_size = ARM_BL_RAM_SIZE;

	/* Initialise the IO layer and register platform IO devices */
	plat_arm_io_setup();
}

void bl2_el3_early_platform_setup(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
	arm_bl2_el3_early_platform_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_arm_interconnect_init();
	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_arm_interconnect_enter_coherency();

	generic_delay_timer_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void arm_bl2_el3_plat_arch_setup(void)
{

#if USE_COHERENT_MEM
	/* Ensure ARM platforms dont use coherent memory in BL2_AT_EL3 */
	assert(BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE == 0U);
#endif

	const mmap_region_t bl_regions[] = {
		MAP_BL2_EL3_TOTAL,
		ARM_MAP_BL_RO,
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());

#ifdef AARCH32
	enable_mmu_svc_mon(0);
#else
	enable_mmu_el3(0);
#endif
}

void bl2_el3_plat_arch_setup(void)
{
	arm_bl2_el3_plat_arch_setup();
}

void bl2_el3_plat_prepare_exit(void)
{
}
