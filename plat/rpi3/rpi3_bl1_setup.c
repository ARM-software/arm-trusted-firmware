/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <bl_common.h>
#include <console.h>
#include <platform_def.h>
#include <xlat_mmu_helpers.h>
#include <xlat_tables_defs.h>

#include "../../bl1/bl1_private.h"
#include "rpi3_private.h"

/* Data structure which holds the extents of the trusted SRAM for BL1 */
static meminfo_t bl1_tzram_layout;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_init(PLAT_RPI3_UART_BASE, PLAT_RPI3_UART_CLK_IN_HZ,
		     PLAT_RPI3_UART_BAUDRATE);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL_RAM_BASE;
	bl1_tzram_layout.total_size = BL_RAM_SIZE;
}

/******************************************************************************
 * Perform the very early platform specific architecture setup.  This only
 * does basic initialization. Later architectural setup (bl1_arch_setup())
 * does not do anything platform specific.
 *****************************************************************************/
void bl1_plat_arch_setup(void)
{
	rpi3_setup_page_tables(bl1_tzram_layout.total_base,
			       bl1_tzram_layout.total_size,
			       BL_CODE_BASE, BL1_CODE_END,
			       BL1_RO_DATA_BASE, BL1_RO_DATA_END
#if USE_COHERENT_MEM
			       , BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END
#endif
			      );

	enable_mmu_el3(0);
}

void bl1_platform_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	plat_rpi3_io_setup();
}
