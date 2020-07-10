/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/console.h>
#include <lib/utils.h>

#ifdef SPD_opteed
#include <optee_utils.h>
#endif
#include <marvell_def.h>
#include <plat_marvell.h>

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/* Weak definitions may be overridden in specific MARVELL standard platform */
#pragma weak bl2_early_platform_setup2
#pragma weak bl2_platform_setup
#pragma weak bl2_plat_arch_setup
#pragma weak bl2_plat_sec_mem_layout

meminfo_t *bl2_plat_sec_mem_layout(void)
{
	return &bl2_tzram_layout;
}

/*****************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe location before its reclaimed by later BL2 functionality.
 *****************************************************************************
 */
void marvell_bl2_early_platform_setup(meminfo_t *mem_layout)
{
	/* Initialize the console to provide early debug support */
	marvell_console_boot_init();

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	/* Initialise the IO layer and register platform IO devices */
	plat_marvell_io_setup();
}


void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			       u_register_t arg2, u_register_t arg3)
{
	struct meminfo *mem_layout = (struct meminfo *)arg1;

	marvell_bl2_early_platform_setup(mem_layout);
}

void bl2_platform_setup(void)
{
	/* Nothing to do */
}

/*****************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 *****************************************************************************
 */
void marvell_bl2_plat_arch_setup(void)
{
	marvell_setup_page_tables(bl2_tzram_layout.total_base,
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
	enable_mmu_el1(0);
}

void bl2_plat_arch_setup(void)
{
	marvell_bl2_plat_arch_setup();
}

int marvell_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

#ifdef SPD_opteed
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif /* SPD_opteed */
	assert(bl_mem_params);

	switch (image_id) {
	case BL32_IMAGE_ID:
#ifdef SPD_opteed
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
					 &pager_mem_params->image_info,
					 &paged_mem_params->image_info);
		if (err != 0)
			WARN("OPTEE header parse error.\n");
#endif /* SPD_opteed */
		bl_mem_params->ep_info.spsr = marvell_get_spsr_for_bl32_entry();
		break;

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = marvell_get_spsr_for_bl33_entry();
		break;
#ifdef SCP_BL2_BASE
	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific */
		err = bl2_plat_handle_scp_bl2(&bl_mem_params->image_info);
		if (err) {
			WARN("Failure in platform-specific handling of SCP_BL2 image.\n");
		}
		break;
#endif
	default:
		/* Do nothing in default case */
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
	return marvell_bl2_handle_post_image_load(image_id);
}

