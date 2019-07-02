/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <plat/arm/common/plat_arm.h>

#include <plat_private.h>
#include <platform_tsp.h>

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
void tsp_early_platform_setup(void)
{
	/*
	 * Register a different console than already in use to display
	 * messages from TSP
	 */
	static console_cdns_t tsp_boot_console;
	(void)console_cdns_register(ZYNQMP_UART_BASE,
				       zynqmp_get_uart_clk(),
				       ZYNQMP_UART_BAUDRATE,
				       &tsp_boot_console);
	console_set_scope(&tsp_boot_console.console,
			  CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_BOOT);

	/* Initialize the platform config for future decision making */
	zynqmp_config_setup();
}

/*******************************************************************************
 * Perform platform specific setup placeholder
 ******************************************************************************/
void tsp_platform_setup(void)
{
	plat_arm_gic_driver_init();
	plat_arm_gic_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the MMU
 ******************************************************************************/
void tsp_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL32_BASE, BL32_END - BL32_BASE,
			MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE, BL_RO_DATA_END - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE),
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE),
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());
	enable_mmu_el1(0);
}
