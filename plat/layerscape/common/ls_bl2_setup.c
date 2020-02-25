/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>

#include "ls_16550.h"
#include "plat_ls.h"
#include "ls_def.h"

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe location before its reclaimed by later BL2 functionality.
 ******************************************************************************/
void ls_bl2_early_platform_setup(meminfo_t *mem_layout)
{
	static console_t console;

	/* Initialize the console to provide early debug support */
	console_ls_16550_register(LS_TF_UART_BASE, LS_TF_UART_CLOCK,
			       LS_TF_UART_BAUDRATE, &console);

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	/* Initialise the IO layer and register platform IO devices */
	plat_ls_io_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void ls_bl2_plat_arch_setup(void)
{
	ls_setup_page_tables(bl2_tzram_layout.total_base,
			      bl2_tzram_layout.total_size,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
#endif
			      );

#ifdef __aarch64__
	enable_mmu_el1(0);
#else
	enable_mmu_svc_mon(0);
#endif
}

void bl2_plat_arch_setup(void)
{
	ls_bl2_plat_arch_setup();
}

int ls_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params);

	switch (image_id) {
#ifdef __aarch64__
	case BL32_IMAGE_ID:
		bl_mem_params->ep_info.spsr = ls_get_spsr_for_bl32_entry();
		break;
#endif

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = ls_get_spsr_for_bl33_entry();
		break;
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return ls_bl2_handle_post_image_load(image_id);
}
