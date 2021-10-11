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
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <platform_def.h>


void cm_prepare_el2_exit(void);

void bl1_run_next_image(const struct entry_point_info *bl_ep_info);

/*******************************************************************************
 * Function to perform late architectural and platform specific initialization.
 * It also queries the platform to load and run next BL image. Only called
 * by the primary cpu after a cold boot.
 ******************************************************************************/
void bl1_transfer_bl33(void)
{
	unsigned int image_id;

	/* Get the image id of next image to load and run. */
	image_id = bl1_plat_get_next_image_id();

#if !ARM_DISABLE_TRUSTED_WDOG
	/* Disable watchdog before leaving BL1 */
	plat_arm_secure_wdt_stop();
#endif

	bl1_run_next_image(&bl1_plat_get_image_desc(image_id)->ep_info);
}

/*******************************************************************************
 * This function locates and loads the BL33 raw binary image in the trusted SRAM.
 * Called by the primary cpu after a cold boot.
 * TODO: Add support for alternative image load mechanism e.g using virtio/elf
 * loader etc.
 ******************************************************************************/
void bl1_load_bl33(void)
{
	image_desc_t *desc;
	image_info_t *info;
	int err;

	/* Get the image descriptor */
	desc = bl1_plat_get_image_desc(BL33_IMAGE_ID);
	assert(desc != NULL);

	/* Get the image info */
	info = &desc->image_info;
	INFO("BL1: Loading BL33\n");

	err = bl1_plat_handle_pre_image_load(BL33_IMAGE_ID);
	if (err != 0) {
		ERROR("Failure in pre image load handling of BL33 (%d)\n", err);
		plat_error_handler(err);
	}

	err = load_auth_image(BL33_IMAGE_ID, info);
	if (err != 0) {
		ERROR("Failed to load BL33 firmware.\n");
		plat_error_handler(err);
	}

	/* Allow platform to handle image information. */
	err = bl1_plat_handle_post_image_load(BL33_IMAGE_ID);
	if (err != 0) {
		ERROR("Failure in post image load handling of BL33 (%d)\n", err);
		plat_error_handler(err);
	}

	NOTICE("BL1: Booting BL33\n");
}

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
 * This function prepares for entry to BL33
 ******************************************************************************/
void bl1_prepare_next_image(unsigned int image_id)
{
	unsigned int mode = MODE_EL1;
	image_desc_t *desc;
	entry_point_info_t *next_bl_ep;

#if CTX_INCLUDE_AARCH32_REGS
	/*
	 * Ensure that the build flag to save AArch32 system registers in CPU
	 * context is not set for AArch64-only platforms.
	 */
	if (el_implemented(1) == EL_IMPL_A64ONLY) {
		ERROR("EL1 supports AArch64-only. Please set build flag %s",
				"CTX_INCLUDE_AARCH32_REGS = 0\n");
		panic();
	}
#endif

	/* Get the image descriptor. */
	desc = bl1_plat_get_image_desc(image_id);
	assert(desc != NULL);

	/* Get the entry point info. */
	next_bl_ep = &desc->ep_info;

	/* FVP-R is only secure */
	assert(GET_SECURITY_STATE(next_bl_ep->h.attr) == SECURE);

	/* Prepare the SPSR for the next BL image. */
	next_bl_ep->spsr = (uint32_t)SPSR_64((uint64_t) mode,
		(uint64_t)MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);

	/* Allow platform to make change */
	bl1_plat_set_ep_info(image_id, next_bl_ep);

	/* Prepare context for the next EL */
	cm_prepare_el2_exit();

	/* Indicate that image is in execution state. */
	desc->state = IMAGE_STATE_EXECUTED;

	print_entry_point_info(next_bl_ep);
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
		assert(MAX_CACHE_LINE_SIZE >= CACHE_WRITEBACK_GRANULE);
	}
#endif /* ENABLE_ASSERTIONS */

	/* Perform remaining generic architectural setup from ELmax */
	bl1_arch_setup();

#if TRUSTED_BOARD_BOOT
	/* Initialize authentication module */
	auth_mod_init();
#endif /* TRUSTED_BOARD_BOOT */

	/* Perform platform setup in BL1. */
	bl1_platform_setup();

	/* Get the image id of next image to load and run. */
	image_id = bl1_plat_get_next_image_id();

	/*
	 * We currently interpret any image id other than
	 * BL2_IMAGE_ID as the start of firmware update.
	 */
	if (image_id == BL33_IMAGE_ID) {
		bl1_load_bl33();
	} else {
		NOTICE("BL1-FWU: *******FWU Process Started*******\n");
	}

	bl1_prepare_next_image(image_id);

	console_flush();

	bl1_transfer_bl33();
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

