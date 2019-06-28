/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <bl32/tsp/platform_tsp.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <plat/arm/common/plat_arm.h>

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak tsp_early_platform_setup
#pragma weak tsp_platform_setup
#pragma weak tsp_plat_arch_setup

#define MAP_BL_TSP_TOTAL	MAP_REGION_FLAT(			\
					BL32_BASE,			\
					BL32_END - BL32_BASE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
static console_pl011_t arm_tsp_runtime_console;

void arm_tsp_early_platform_setup(void)
{
	/*
	 * Initialize a different console than already in use to display
	 * messages from TSP
	 */
	int rc = console_pl011_register(PLAT_ARM_TSP_UART_BASE,
					PLAT_ARM_TSP_UART_CLK_IN_HZ,
					ARM_CONSOLE_BAUDRATE,
					&arm_tsp_runtime_console);
	if (rc == 0)
		panic();

	console_set_scope(&arm_tsp_runtime_console.console,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);
}

void tsp_early_platform_setup(void)
{
	arm_tsp_early_platform_setup();
}

/*******************************************************************************
 * Perform platform specific setup placeholder
 ******************************************************************************/
void tsp_platform_setup(void)
{
	plat_arm_gic_driver_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the MMU
 ******************************************************************************/
void tsp_plat_arch_setup(void)
{
#if USE_COHERENT_MEM
	/* Ensure ARM platforms don't use coherent memory in TSP */
	assert((BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE) == 0U);
#endif

	const mmap_region_t bl_regions[] = {
		MAP_BL_TSP_TOTAL,
		ARM_MAP_BL_RO,
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());
	enable_mmu_el1(0);
}
