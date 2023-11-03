/*
 * Copyright (c) 2014-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, Advanced Micro Devices. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_tsp.h>

#include <plat_private.h>

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
void tsp_early_platform_setup(void)
{
	/*
	 * Register a different console than already in use to display
	 * messages from TSP
	 */
	static console_t tsp_boot_console;
	int32_t rc;

#if defined(PLAT_zynqmp)
	rc = console_cdns_register((uintptr_t)UART_BASE,
				   (uint32_t)get_uart_clk(),
				   (uint32_t)UART_BAUDRATE,
				   &tsp_boot_console);
#else
	rc = console_pl011_register((uintptr_t)UART_BASE,
				    (uint32_t)get_uart_clk(),
				    (uint32_t)UART_BAUDRATE,
				    &tsp_boot_console);
#endif

	if (rc == 0) {
		panic();
	}

	console_set_scope(&tsp_boot_console,
			  CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_BOOT);
}

/*******************************************************************************
 * Perform platform specific setup placeholder
 ******************************************************************************/
void tsp_platform_setup(void)
{
/*
 * For ZynqMP, the GICv2 driver needs to be initialized in S-EL1,
 * and for other platforms, the GICv3 driver is initialized in EL3.
 * This is because S-EL1 can use GIC system registers to manage
 * interrupts and does not need to be initialized again in SEL1.
 */
#if defined(PLAT_zynqmp)
	plat_arm_gic_driver_init();
	plat_arm_gic_init();
#endif
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the MMU
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
#if defined(PLAT_zynqmp) || defined(PLAT_versal)
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE),
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_get_mmap());
	enable_mmu_el1(0);
}
