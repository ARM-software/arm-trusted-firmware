/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <bl1.h>
#include <bl1/bl1_private.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include <plat_marvell.h>
#include <sp805.h>

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
	const size_t bl1_size = BL1_RAM_LIMIT - BL1_RAM_BASE;

	/* Initialize the console to provide early debug support */
	console_init(PLAT_MARVELL_BOOT_UART_BASE,
		     PLAT_MARVELL_BOOT_UART_CLK_IN_HZ,
		     MARVELL_CONSOLE_BAUDRATE);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_ram_layout.total_base = MARVELL_BL_RAM_BASE;
	bl1_ram_layout.total_size = MARVELL_BL_RAM_SIZE;

	/* Calculate how much RAM BL1 is using and how much remains free */
	bl1_ram_layout.free_base = MARVELL_BL_RAM_BASE;
	bl1_ram_layout.free_size = MARVELL_BL_RAM_SIZE;
	reserve_mem(&bl1_ram_layout.free_base,
		    &bl1_ram_layout.free_size,
		    BL1_RAM_BASE,
		    bl1_size);
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
