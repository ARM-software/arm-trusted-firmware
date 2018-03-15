/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <ls_def.h>
#include <arm_xlat_tables.h>
#include <bl_common.h>
#include <console.h>
#include <platform_def.h>
#include <plat_ls.h>
#include <utils.h>
#include "../../../bl1/bl1_private.h"
#include <debug.h>
#include <uart_16550.h>

/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo_t bl1_tzram_layout;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*******************************************************************************
 * BL1 specific platform actions shared between ARM standard platforms.
 ******************************************************************************/
void ls_bl1_early_platform_setup(void)
{
	static console_16550_t console;

#if !LS1043_DISABLE_TRUSTED_WDOG
	/* TODO: Enable watchdog */

#endif

	/* Initialize the console to provide early debug support */
	console_16550_register(LS_ATF_UART_BASE, LS_ATF_UART_CLOCK,
			       LS_ATF_UART_BAUDRATE, &console);
#ifdef NOR_BOOT
	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = LS_SRAM_BASE;
	bl1_tzram_layout.total_size = LS_SRAM_SIZE;
#else
	/* For SD boot, Don't include BL1 RO region which is at the start of OCRAM */
	bl1_tzram_layout.total_base = BL31_BASE;
	bl1_tzram_layout.total_size = LS_SRAM_SIZE - PLAT_LS_MAX_BL1_RO_SIZE;
#endif

#if !LOAD_IMAGE_V2
	/* Calculate how much RAM BL1 is using and how much remains free */
	bl1_tzram_layout.free_base = LS_BL_RAM_BASE;
	bl1_tzram_layout.free_size = LS_SRAM_SIZE - LS_SRAM_SHARED_SIZE;
	reserve_mem(&bl1_tzram_layout.free_base,
		    &bl1_tzram_layout.free_size,
		    BL1_RAM_BASE,
		    BL1_RAM_LIMIT - BL1_RAM_BASE);
#endif /* LOAD_IMAGE_V2 */
}

/******************************************************************************
 * Perform the very early platform specific architecture setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl1_arch_setup()) does not do anything platform
 * specific.
 *****************************************************************************/
void ls_bl1_plat_arch_setup(void)
{
	ls_setup_page_tables(bl1_tzram_layout.total_base,
			      bl1_tzram_layout.total_size,
			      BL_CODE_BASE,
			      BL1_CODE_END,
			      BL1_RO_DATA_BASE,
			      BL1_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
#endif
			     );
	VERBOSE("After setup the page tables\n");
#ifdef AARCH32
	enable_mmu_secure(0);
#else
	enable_mmu_el3(0);
#endif /* AARCH32 */
	VERBOSE("After MMU enabled\n");
}

void bl1_plat_arch_setup(void)
{
	ls_bl1_plat_arch_setup();
}

/*
 * Perform the platform specific architecture setup shared between
 * ARM standard platforms.
 */
void ls_bl1_platform_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	plat_ls_io_setup();
}

void bl1_plat_prepare_exit(entry_point_info_t *ep_info)
{
#if !LS1043_DISABLE_TRUSTED_WDOG
	/*TODO: Disable watchdog before leaving BL1 */
#endif
}
