/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <lib/mmio.h>

#include <bcm_console.h>
#include <platform_def.h>
#include <plat/brcm/common/plat_brcm.h>

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/* Weak definitions may be overridden in specific BRCM platform */
#pragma weak plat_bcm_bl2_platform_setup
#pragma weak plat_bcm_bl2_plat_arch_setup
#pragma weak plat_bcm_security_setup
#pragma weak plat_bcm_bl2_plat_handle_scp_bl2
#pragma weak plat_bcm_bl2_early_platform_setup

void plat_bcm_bl2_early_platform_setup(void)
{
}

void plat_bcm_bl2_platform_setup(void)
{
}

void plat_bcm_bl2_plat_arch_setup(void)
{
}

void plat_bcm_security_setup(void)
{
}

void bcm_bl2_early_platform_setup(uintptr_t tb_fw_config,
				  meminfo_t *mem_layout)
{
	/* Initialize the console to provide early debug support */
	bcm_console_boot_init();

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	/* Initialise the IO layer and register platform IO devices */
	plat_brcm_io_setup();

	/* Log HW reset event */
	INFO("RESET: 0x%x\n",
		mmio_read_32(CRMU_RESET_EVENT_LOG));
}

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			       u_register_t arg2, u_register_t arg3)
{
	/* SoC specific setup */
	plat_bcm_bl2_early_platform_setup();

	/* Initialize delay timer driver using SP804 dual timer 0 */
	sp804_timer_init(SP804_TIMER0_BASE,
			 SP804_TIMER0_CLKMULT, SP804_TIMER0_CLKDIV);

	/* BRCM platforms generic setup */
	bcm_bl2_early_platform_setup((uintptr_t)arg0, (meminfo_t *)arg1);
}

/*
 * Perform Broadcom platform setup.
 */
void bcm_bl2_platform_setup(void)
{
	/* Initialize the secure environment */
	plat_bcm_security_setup();
}

void bl2_platform_setup(void)
{
	bcm_bl2_platform_setup();
	plat_bcm_bl2_platform_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bcm_bl2_plat_arch_setup(void)
{
#ifndef MMU_DISABLED
	if (!(read_sctlr_el1() & SCTLR_M_BIT)) {
		const mmap_region_t bl_regions[] = {
			MAP_REGION_FLAT(bl2_tzram_layout.total_base,
					bl2_tzram_layout.total_size,
					MT_MEMORY | MT_RW | MT_SECURE),
			MAP_REGION_FLAT(BL_CODE_BASE,
					BL_CODE_END - BL_CODE_BASE,
					MT_CODE | MT_SECURE),
			MAP_REGION_FLAT(BL_RO_DATA_BASE,
					BL_RO_DATA_END - BL_RO_DATA_BASE,
					MT_RO_DATA | MT_SECURE),
#if USE_COHERENT_MEM
			MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
					BL_COHERENT_RAM_END -
					BL_COHERENT_RAM_BASE,
					MT_DEVICE | MT_RW | MT_SECURE),
#endif
			{0}
		};

		setup_page_tables(bl_regions, plat_brcm_get_mmap());
		enable_mmu_el1(0);
	}
#endif
}

void bl2_plat_arch_setup(void)
{
#ifdef ENA_MMU_BEFORE_DDR_INIT
	/*
	 * Once MMU is enabled before DDR, MEMORY TESTS
	 * get affected as read/write transaction might occures from
	 * caches. So For running memory test, one should not set this
	 * flag.
	 */
	bcm_bl2_plat_arch_setup();
	plat_bcm_bl2_plat_arch_setup();
#else
	plat_bcm_bl2_plat_arch_setup();
	bcm_bl2_plat_arch_setup();
#endif
}

int bcm_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;

	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params);

	switch (image_id) {
	case BL32_IMAGE_ID:
		bl_mem_params->ep_info.spsr = brcm_get_spsr_for_bl32_entry();
		break;

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = brcm_get_spsr_for_bl33_entry();
		break;

#ifdef SCP_BL2_BASE
	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific */
		err = bcm_bl2_handle_scp_bl2(&bl_mem_params->image_info);
		if (err)
			WARN("Failure in platform-specific handling of SCP_BL2 image.\n");
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
int bcm_bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return bcm_bl2_handle_post_image_load(image_id);
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return bcm_bl2_plat_handle_post_image_load(image_id);
}

#ifdef SCP_BL2_BASE
int plat_bcm_bl2_plat_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	return 0;
}

int bcm_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	return plat_bcm_bl2_plat_handle_scp_bl2(scp_bl2_image_info);
}
#endif
