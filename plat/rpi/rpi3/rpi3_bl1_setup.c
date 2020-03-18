/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#include <rpi_shared.h>

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
	/* use the 19.2 MHz clock for the architected timer */
	mmio_write_32(RPI3_INTC_BASE_ADDRESS + RPI3_INTC_CONTROL_OFFSET, 0);
	mmio_write_32(RPI3_INTC_BASE_ADDRESS + RPI3_INTC_PRESCALER_OFFSET,
		      0x80000000);

	/* Initialize the console to provide early debug support */
	rpi3_console_init();

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
	uint32_t __unused rev;
	int __unused rc;

	rc = rpi3_vc_hardware_get_board_revision(&rev);

	if (rc == 0) {
		const char __unused *model, __unused *info;

		switch (rev) {
		case 0xA02082:
			model = "Raspberry Pi 3 Model B";
			info = "(1GB, Sony, UK)";
			break;
		case 0xA22082:
			model = "Raspberry Pi 3 Model B";
			info = "(1GB, Embest, China)";
			break;
		case 0xA020D3:
			model = "Raspberry Pi 3 Model B+";
			info = "(1GB, Sony, UK)";
			break;
		default:
			model = "Unknown";
			info = "(Unknown)";
			ERROR("rpi3: Unknown board revision 0x%08x\n", rev);
			break;
		}

		NOTICE("rpi3: Detected: %s %s [0x%08x]\n", model, info, rev);
	} else {
		ERROR("rpi3: Unable to detect board revision\n");
	}

	/* Initialise the IO layer and register platform IO devices */
	plat_rpi3_io_setup();
}
