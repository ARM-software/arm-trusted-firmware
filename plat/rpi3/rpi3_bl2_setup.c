/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <desc_image_load.h>
#include <platform_def.h>
#include <xlat_mmu_helpers.h>
#include <xlat_tables_defs.h>

#include "rpi3_private.h"

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe location before its reclaimed by later BL2 functionality.
 ******************************************************************************/
void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	/* Initialize the console to provide early debug support */
	console_init(PLAT_RPI3_UART_BASE, PLAT_RPI3_UART_CLK_IN_HZ,
		     PLAT_RPI3_UART_BAUDRATE);

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	plat_rpi3_io_setup();
}

void bl2_platform_setup(void)
{
	/*
	 * This is where a TrustZone address space controller and other
	 * security related peripherals, would be configured.
	 */
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here.
 ******************************************************************************/
void bl2_plat_arch_setup(void)
{
	rpi3_setup_page_tables(bl2_tzram_layout.total_base,
			       bl2_tzram_layout.total_size,
			       BL_CODE_BASE, BL_CODE_END,
			       BL_RO_DATA_BASE, BL_RO_DATA_END
#if USE_COHERENT_MEM
			       , BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END
#endif
			      );

	enable_mmu_el1(0);
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params != NULL);

	switch (image_id) {
	case BL32_IMAGE_ID:
		bl_mem_params->ep_info.spsr = rpi3_get_spsr_for_bl32_entry();
		break;

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = rpi3_get_spsr_for_bl33_entry();
		break;

	}

	return err;
}
