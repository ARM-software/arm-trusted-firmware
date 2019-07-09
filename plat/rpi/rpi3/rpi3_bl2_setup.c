/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <lib/optee_utils.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/rpi3/gpio/rpi3_gpio.h>
#include <drivers/rpi3/sdhost/rpi3_sdhost.h>

#include "rpi3_private.h"

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/* rpi3 GPIO setup function. */
static void rpi3_gpio_setup(void)
{
	struct rpi3_gpio_params params;

	memset(&params, 0, sizeof(struct rpi3_gpio_params));
	params.reg_base = RPI3_GPIO_BASE;

	rpi3_gpio_init(&params);
}

/* Data structure which holds the MMC info */
static struct mmc_device_info mmc_info;

static void rpi3_sdhost_setup(void)
{
	struct rpi3_sdhost_params params;

	memset(&params, 0, sizeof(struct rpi3_sdhost_params));
	params.reg_base = RPI3_SDHOST_BASE;
	params.bus_width = MMC_BUS_WIDTH_1;
	params.clk_rate = 50000000;
	mmc_info.mmc_dev_type = MMC_IS_SD_HC;
	rpi3_sdhost_init(&params, &mmc_info);
}

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe location before its reclaimed by later BL2 functionality.
 ******************************************************************************/

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			       u_register_t arg2, u_register_t arg3)
{
	meminfo_t *mem_layout = (meminfo_t *) arg1;

	/* Initialize the console to provide early debug support */
	rpi3_console_init();

	/* Enable arch timer */
	generic_delay_timer_init();

	/* Setup GPIO driver */
	rpi3_gpio_setup();

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	/* Setup SDHost driver */
	rpi3_sdhost_setup();

	plat_rpi3_io_setup();
}

void bl2_platform_setup(void)
{
	/*
	 * This is where a TrustZone address space controller and other
	 * security related peripherals would be configured.
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
#ifdef SPD_opteed
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif

	assert(bl_mem_params != NULL);

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
#endif
		bl_mem_params->ep_info.spsr = rpi3_get_spsr_for_bl32_entry();
		break;

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = rpi3_get_spsr_for_bl33_entry();

		/* Shutting down the SDHost driver to let BL33 drives SDHost.*/
		rpi3_sdhost_stop();
		break;

	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}
