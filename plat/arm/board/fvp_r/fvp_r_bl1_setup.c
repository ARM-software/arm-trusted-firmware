/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Use the xlat_tables_v2 data structures: */
#define XLAT_TABLES_LIB_V2	1

#include <assert.h>

#include <bl1/bl1.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/sp805.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/xlat_mpu/xlat_mpu.h>

#include "fvp_r_private.h"
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/arm_def.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#define MAP_BL1_TOTAL		MAP_REGION_FLAT(			\
					bl1_tzram_layout.total_base,	\
					bl1_tzram_layout.total_size,	\
					MT_MEMORY | MT_RW | MT_SECURE)
/*
 * If SEPARATE_CODE_AND_RODATA=1 we define a region for each section
 * otherwise one region is defined containing both
 */
#if SEPARATE_CODE_AND_RODATA
#define MAP_BL1_RO		MAP_REGION_FLAT(			\
					BL_CODE_BASE,			\
					BL1_CODE_END - BL_CODE_BASE,	\
					MT_CODE | MT_SECURE),		\
				MAP_REGION_FLAT(			\
					BL1_RO_DATA_BASE,		\
					BL1_RO_DATA_END			\
						- BL_RO_DATA_BASE,	\
					MT_RO_DATA | MT_SECURE)
#else
#define MAP_BL1_RO		MAP_REGION_FLAT(			\
					BL_CODE_BASE,			\
					BL1_CODE_END - BL_CODE_BASE,	\
					MT_CODE | MT_SECURE)
#endif

/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo_t bl1_tzram_layout;

struct meminfo *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

void arm_bl1_early_platform_setup(void)
{

#if !ARM_DISABLE_TRUSTED_WDOG
	/* Enable watchdog */
	plat_arm_secure_wdt_start();
#endif

	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = ARM_BL_RAM_BASE;
	bl1_tzram_layout.total_size = ARM_BL_RAM_SIZE;
}

/* Boolean variable to hold condition whether firmware update needed or not */
static bool is_fwu_needed;

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	arm_bl1_early_platform_setup();

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();
	/*
	 * Enable coherency in Interconnect for the primary CPU's cluster.
	 */
	fvp_interconnect_enable();
}

void arm_bl1_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL1_TOTAL,
		MAP_BL1_RO,
#if USE_ROMLIB
		ARM_MAP_ROMLIB_CODE,
		ARM_MAP_ROMLIB_DATA,
#endif
#if ARM_CRYPTOCELL_INTEG
		ARM_MAP_BL_COHERENT_RAM,
#endif
		/* DRAM1_region: */
		MAP_REGION_FLAT(					\
			PLAT_ARM_DRAM1_BASE,				\
			PLAT_ARM_DRAM1_SIZE,				\
			MT_MEMORY | MT_SECURE | MT_EXECUTE		\
			| MT_RW | MT_NON_CACHEABLE),
		/* NULL terminator: */
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());
	enable_mpu_el2(0);

	arm_setup_romlib();
}

void plat_arm_secure_wdt_start(void)
{
	sp805_start(ARM_SP805_TWDG_BASE, ARM_TWDG_LOAD_VAL);
}

void plat_arm_secure_wdt_stop(void)
{
	sp805_stop(ARM_SP805_TWDG_BASE);
}

/*
 * Perform the platform specific architecture setup shared between
 * ARM standard platforms.
 */
void arm_bl1_platform_setup(void)
{
	uint32_t fw_config_max_size;

	/* Initialise the IO layer and register platform IO devices */
	plat_arm_io_setup();

	/* Check if we need FWU before further processing */
	is_fwu_needed = plat_arm_bl1_fwu_needed();
	if (is_fwu_needed) {
		ERROR("Skip platform setup as FWU detected\n");
		return;
	}

	/* Set global DTB info for fixed fw_config information */
	fw_config_max_size = ARM_FW_CONFIG_LIMIT - ARM_FW_CONFIG_BASE;
	set_config_info(ARM_FW_CONFIG_BASE, fw_config_max_size, FW_CONFIG_ID);

	assert(bl1_plat_get_image_desc(BL33_IMAGE_ID) != NULL);

	/*
	 * Allow access to the System counter timer module and program
	 * counter frequency for non secure images during FWU
	 */
#ifdef ARM_SYS_TIMCTL_BASE
	arm_configure_sys_timer();
#endif
#if (ARM_ARCH_MAJOR > 7) || defined(ARMV7_SUPPORTS_GENERIC_TIMER)
	write_cntfrq_el0(plat_get_syscnt_freq2());
#endif
}

void bl1_platform_setup(void)
{
	arm_bl1_platform_setup();

	/* Initialize System level generic or SP804 timer */
	fvp_timer_init();
}

__dead2 void bl1_plat_fwu_done(void *client_cookie, void *reserved)
{
	/* Setup the watchdog to reset the system as soon as possible */
	sp805_refresh(ARM_SP805_TWDG_BASE, 1U);

	while (true) {
		wfi();
	}
}

unsigned int bl1_plat_get_next_image_id(void)
{
	return  is_fwu_needed ? NS_BL1U_IMAGE_ID : BL33_IMAGE_ID;
}

/*
 * Returns BL33 image details.
 */
struct image_desc *bl1_plat_get_image_desc(unsigned int image_id)
{
	static image_desc_t bl33_img_desc = BL33_IMAGE_DESC;

	return &bl33_img_desc;
}

/*
 * This function populates the default arguments to BL33.
 * The BL33 memory layout structure is allocated and the
 * calculated layout is populated in arg1 to BL33.
 */
int bl1_plat_handle_post_image_load(unsigned int image_id)
{
	meminfo_t *bl33_secram_layout;
	meminfo_t *bl1_secram_layout;
	image_desc_t *image_desc;
	entry_point_info_t *ep_info;

	if (image_id != BL33_IMAGE_ID) {
		return 0;
	}
	/* Get the image descriptor */
	image_desc = bl1_plat_get_image_desc(BL33_IMAGE_ID);
	assert(image_desc != NULL);

	/* Get the entry point info */
	ep_info = &image_desc->ep_info;

	/* Find out how much free trusted ram remains after BL1 load */
	bl1_secram_layout = bl1_plat_sec_mem_layout();

	/*
	 * Create a new layout of memory for BL33 as seen by BL1 i.e.
	 * tell it the amount of total and free memory available.
	 * This layout is created at the first free address visible
	 * to BL33. BL33 will read the memory layout before using its
	 * memory for other purposes.
	 */
	bl33_secram_layout = (meminfo_t *) bl1_secram_layout->total_base;

	bl1_calc_bl2_mem_layout(bl1_secram_layout, bl33_secram_layout);

	ep_info->args.arg1 = (uintptr_t)bl33_secram_layout;

	VERBOSE("BL1: BL3 memory layout address = %p\n",
		(void *) bl33_secram_layout);
	return 0;
}
