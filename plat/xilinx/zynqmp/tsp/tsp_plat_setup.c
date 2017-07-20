/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <plat_arm.h>
#include <platform_tsp.h>
#include "../zynqmp_private.h"

#define BL32_END (unsigned long)(&__BL32_END__)

/*******************************************************************************
 * Initialize the UART
 ******************************************************************************/
void tsp_early_platform_setup(void)
{
	/*
	 * Initialize a different console than already in use to display
	 * messages from TSP
	 */
	console_init(ZYNQMP_UART_BASE, zynqmp_get_uart_clk(),
		     ZYNQMP_UART_BAUDRATE);

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
	arm_setup_page_tables(BL32_BASE,
			      BL32_END - BL32_BASE,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END,
			      BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
			      );
	enable_mmu_el1(0);
}
