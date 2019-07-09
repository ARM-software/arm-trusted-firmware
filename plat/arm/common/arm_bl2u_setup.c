/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <drivers/generic_delay_timer.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak bl2u_platform_setup
#pragma weak bl2u_early_platform_setup
#pragma weak bl2u_plat_arch_setup

#define MAP_BL2U_TOTAL		MAP_REGION_FLAT(			\
					BL2U_BASE,			\
					BL2U_LIMIT - BL2U_BASE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

/*
 * Perform ARM standard platform setup for BL2U
 */
void arm_bl2u_platform_setup(void)
{
	/* Initialize the secure environment */
	plat_arm_security_setup();
}

void bl2u_platform_setup(void)
{
	arm_bl2u_platform_setup();
}

void arm_bl2u_early_platform_setup(struct meminfo *mem_layout, void *plat_info)
{
	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

	generic_delay_timer_init();
}

/*******************************************************************************
 * BL1 can pass platform dependent information to BL2U in x1.
 * In case of ARM CSS platforms x1 contains SCP_BL2U image info.
 * In case of ARM FVP platforms x1 is not used.
 * In both cases, x0 contains the extents of the memory available to BL2U
 ******************************************************************************/
void bl2u_early_platform_setup(struct meminfo *mem_layout, void *plat_info)
{
	arm_bl2u_early_platform_setup(mem_layout, plat_info);
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 * The memory that is used by BL2U is only mapped.
 ******************************************************************************/
void arm_bl2u_plat_arch_setup(void)
{

#if USE_COHERENT_MEM
	/* Ensure ARM platforms dont use coherent memory in BL2U */
	assert((BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE) == 0U);
#endif

	const mmap_region_t bl_regions[] = {
		MAP_BL2U_TOTAL,
		ARM_MAP_BL_RO,
#if USE_ROMLIB
		ARM_MAP_ROMLIB_CODE,
		ARM_MAP_ROMLIB_DATA,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());

#ifdef __aarch64__
	enable_mmu_el1(0);
#else
	enable_mmu_svc_mon(0);
#endif
	arm_setup_romlib();
}

void bl2u_plat_arch_setup(void)
{
	arm_bl2u_plat_arch_setup();
}
