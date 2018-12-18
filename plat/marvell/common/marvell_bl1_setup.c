/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <platform_def.h>

#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/sp805.h>
#include <drivers/console.h>
#include <plat/common/platform.h>

#include <plat_marvell.h>

/* Weak definitions may be overridden in specific Marvell standard platform */
#pragma weak bl1_early_platform_setup
#pragma weak bl1_plat_arch_setup
#pragma weak bl1_platform_setup
#pragma weak bl1_plat_sec_mem_layout

/* Data structure which holds the extents of the RAM for BL1*/
static meminfo_t bl1_ram_layout;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_ram_layout;
}

/*
 * BL1 specific platform actions shared between Marvell standard platforms.
 */
void marvell_bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	marvell_console_boot_init();

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_ram_layout.total_base = MARVELL_BL_RAM_BASE;
	bl1_ram_layout.total_size = MARVELL_BL_RAM_SIZE;
}

void bl1_early_platform_setup(void)
{
	marvell_bl1_early_platform_setup();
}

/*
 * Perform the very early platform specific architecture setup shared between
 * MARVELL standard platforms. This only does basic initialization. Later
 * architectural setup (bl1_arch_setup()) does not do anything platform
 * specific.
 */
void marvell_bl1_plat_arch_setup(void)
{
	marvell_setup_page_tables(bl1_ram_layout.total_base,
				  bl1_ram_layout.total_size,
				  BL1_RO_BASE,
				  BL1_RO_LIMIT,
				  BL1_RO_DATA_BASE,
				  BL1_RO_DATA_END
#if USE_COHERENT_MEM
				, BL_COHERENT_RAM_BASE,
				  BL_COHERENT_RAM_END
#endif
				);
	enable_mmu_el3(0);
}

void bl1_plat_arch_setup(void)
{
	marvell_bl1_plat_arch_setup();
}

/*
 * Perform the platform specific architecture setup shared between
 * MARVELL standard platforms.
 */
void marvell_bl1_platform_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	plat_marvell_io_setup();
}

void bl1_platform_setup(void)
{
	marvell_bl1_platform_setup();
}

void bl1_plat_prepare_exit(entry_point_info_t *ep_info)
{
#ifdef EL3_PAYLOAD_BASE
	/*
	 * Program the EL3 payload's entry point address into the CPUs mailbox
	 * in order to release secondary CPUs from their holding pen and make
	 * them jump there.
	 */
	marvell_program_trusted_mailbox(ep_info->pc);
	dsbsy();
	sev();
#endif
}
