/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include "../../../../bl1/bl1_private.h"
#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/console.h>
#include <lib/cpus/errata_report.h>
#include <lib/utils.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>
#include <plat/common/platform.h>

#include <platform_def.h>


static void bl1_load_bl2(void);

#if ENABLE_PAUTH
uint64_t bl1_apiakey[2];
#endif

/*******************************************************************************
 * Helper utility to calculate the BL2 memory layout taking into consideration
 * the BL1 RW data assuming that it is at the top of the memory layout.
 ******************************************************************************/
void bl1_calc_bl2_mem_layout(const meminfo_t *bl1_mem_layout,
			meminfo_t *bl2_mem_layout)
{
	assert(bl1_mem_layout != NULL);
	assert(bl2_mem_layout != NULL);

	/*
	 * Remove BL1 RW data from the scope of memory visible to BL2.
	 * This is assuming BL1 RW data is at the top of bl1_mem_layout.
	 */
	assert(bl1_mem_layout->total_base < BL1_RW_BASE);
	bl2_mem_layout->total_base = bl1_mem_layout->total_base;
	bl2_mem_layout->total_size = BL1_RW_BASE - bl1_mem_layout->total_base;

	flush_dcache_range((uintptr_t)bl2_mem_layout, sizeof(meminfo_t));
}

/*******************************************************************************
 * Setup function for BL1.
 ******************************************************************************/
void bl1_setup(void)
{
	/* Perform early platform-specific setup */
	bl1_early_platform_setup();

	/* Perform late platform-specific setup */
	bl1_plat_arch_setup();

#if CTX_INCLUDE_PAUTH_REGS
	/*
	 * Assert that the ARMv8.3-PAuth registers are present or an access
	 * fault will be triggered when they are being saved or restored.
	 */
	assert(is_armv8_3_pauth_present());
#endif /* CTX_INCLUDE_PAUTH_REGS */
}

/*******************************************************************************
 * Function to perform late architectural and platform specific initialization.
 * It also queries the platform to load and run next BL image. Only called
 * by the primary cpu after a cold boot.
 ******************************************************************************/
void bl1_main(void)
{
	unsigned int image_id;

	/* Announce our arrival */
	NOTICE(FIRMWARE_WELCOME_STR);
	NOTICE("BL1: %s\n", version_string);
	NOTICE("BL1: %s\n", build_message);

	INFO("BL1: RAM %p - %p\n", (void *)BL1_RAM_BASE, (void *)BL1_RAM_LIMIT);

	print_errata_status();

#if ENABLE_ASSERTIONS
	u_register_t val;
	/*
	 * Ensure that MMU/Caches and coherency are turned on
	 */
	val = read_sctlr_el2();

	assert((val & SCTLR_M_BIT) != 0U);
	assert((val & SCTLR_C_BIT) != 0U);
	assert((val & SCTLR_I_BIT) != 0U);
	/*
	 * Check that Cache Writeback Granule (CWG) in CTR_EL0 matches the
	 * provided platform value
	 */
	val = (read_ctr_el0() >> CTR_CWG_SHIFT) & CTR_CWG_MASK;
	/*
	 * If CWG is zero, then no CWG information is available but we can
	 * at least check the platform value is less than the architectural
	 * maximum.
	 */
	if (val != 0) {
		assert(SIZE_FROM_LOG2_WORDS(val) == CACHE_WRITEBACK_GRANULE);
	} else {
		assert(CACHE_WRITEBACK_GRANULE <= MAX_CACHE_LINE_SIZE);
	}
#endif /* ENABLE_ASSERTIONS */

	/* Perform remaining generic architectural setup from EL2 */
	bl1_arch_setup();

#if TRUSTED_BOARD_BOOT
	/* Initialize authentication module */
	auth_mod_init();
#endif /* TRUSTED_BOARD_BOOT */

	/* Perform platform setup in BL1. */
	bl1_platform_setup();

#if ENABLE_PAUTH
	/* Store APIAKey_EL1 key */
	bl1_apiakey[0] = read_apiakeylo_el1();
	bl1_apiakey[1] = read_apiakeyhi_el1();
#endif /* ENABLE_PAUTH */

	/* Get the image id of next image to load and run. */
	image_id = bl1_plat_get_next_image_id();

	/*
	 * We currently interpret any image id other than
	 * BL2_IMAGE_ID as the start of firmware update.
	 */
	if (image_id == BL2_IMAGE_ID) {
		bl1_load_bl2();
	} else {
		NOTICE("BL1-FWU: *******FWU Process Started*******\n");
	}
	bl1_prepare_next_image(image_id);

	console_flush();
}

/*******************************************************************************
 * This function locates and loads the BL2 raw binary image in the trusted SRAM.
 * Called by the primary cpu after a cold boot.
 * TODO: Add support for alternative image load mechanism e.g using virtio/elf
 * loader etc.
 ******************************************************************************/
static void bl1_load_bl2(void)
{
	image_desc_t *desc;
	image_info_t *info;
	int err;

	/* Get the image descriptor */
	desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(desc != NULL);

	/* Get the image info */
	info = &desc->image_info;
	INFO("BL1: Loading BL2\n");

	err = bl1_plat_handle_pre_image_load(BL2_IMAGE_ID);
	if (err != 0) {
		ERROR("Failure in pre image load handling of BL2 (%d)\n", err);
		plat_error_handler(err);
	}

	err = load_auth_image(BL2_IMAGE_ID, info);
	if (err != 0) {
		ERROR("Failed to load BL2 firmware.\n");
		plat_error_handler(err);
	}

	/* Allow platform to handle image information. */
	err = bl1_plat_handle_post_image_load(BL2_IMAGE_ID);
	if (err != 0) {
		ERROR("Failure in post image load handling of BL2 (%d)\n", err);
		plat_error_handler(err);
	}

	NOTICE("BL1: Booting BL2\n");
}

/*******************************************************************************
 * Function called just before handing over to the next BL to inform the user
 * about the boot progress. In debug mode, also print details about the BL
 * image's execution context.
 ******************************************************************************/
void bl1_print_next_bl_ep_info(const entry_point_info_t *bl_ep_info)
{
	NOTICE("BL1: Booting BL31\n");
	print_entry_point_info(bl_ep_info);
}

#if SPIN_ON_BL1_EXIT
void print_debug_loop_message(void)
{
	NOTICE("BL1: Debug loop, spinning forever\n");
	NOTICE("BL1: Please connect the debugger to continue\n");
}
#endif

