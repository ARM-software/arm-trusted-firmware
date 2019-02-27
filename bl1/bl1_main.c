/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/console.h>
#include <lib/cpus/errata_report.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>

#include "bl1_private.h"

/* BL1 Service UUID */
DEFINE_SVC_UUID2(bl1_svc_uid,
	0xd46739fd, 0xcb72, 0x9a4d, 0xb5, 0x75,
	0x67, 0x15, 0xd6, 0xf4, 0xbb, 0x4a);

static void bl1_load_bl2(void);

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
	assert(BL1_RW_BASE > bl1_mem_layout->total_base);
	bl2_mem_layout->total_base = bl1_mem_layout->total_base;
	bl2_mem_layout->total_size = BL1_RW_BASE - bl1_mem_layout->total_base;

	flush_dcache_range((unsigned long)bl2_mem_layout, sizeof(meminfo_t));
}

/*******************************************************************************
 * Setup function for BL1.
 ******************************************************************************/
void bl1_setup(void)
{
	/* Perform early platform-specific setup */
	bl1_early_platform_setup();

#ifdef AARCH64
	/*
	 * Update pointer authentication key before the MMU is enabled. It is
	 * saved in the rodata section, that can be writen before enabling the
	 * MMU. This function must be called after the console is initialized
	 * in the early platform setup.
	 */
	bl_handle_pauth();
#endif /* AARCH64 */

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

	INFO("BL1: RAM %p - %p\n", (void *)BL1_RAM_BASE,
					(void *)BL1_RAM_LIMIT);

	print_errata_status();

#if ENABLE_ASSERTIONS
	u_register_t val;
	/*
	 * Ensure that MMU/Caches and coherency are turned on
	 */
#ifdef AARCH32
	val = read_sctlr();
#else
	val = read_sctlr_el3();
#endif
	assert(val & SCTLR_M_BIT);
	assert(val & SCTLR_C_BIT);
	assert(val & SCTLR_I_BIT);
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
	if (val != 0)
		assert(CACHE_WRITEBACK_GRANULE == SIZE_FROM_LOG2_WORDS(val));
	else
		assert(CACHE_WRITEBACK_GRANULE <= MAX_CACHE_LINE_SIZE);
#endif /* ENABLE_ASSERTIONS */

	/* Perform remaining generic architectural setup from EL3 */
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
	if (image_id == BL2_IMAGE_ID)
		bl1_load_bl2();
	else
		NOTICE("BL1-FWU: *******FWU Process Started*******\n");

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
	image_desc_t *image_desc;
	image_info_t *image_info;
	int err;

	/* Get the image descriptor */
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc);

	/* Get the image info */
	image_info = &image_desc->image_info;
	INFO("BL1: Loading BL2\n");

	err = bl1_plat_handle_pre_image_load(BL2_IMAGE_ID);
	if (err) {
		ERROR("Failure in pre image load handling of BL2 (%d)\n", err);
		plat_error_handler(err);
	}

	err = load_auth_image(BL2_IMAGE_ID, image_info);
	if (err) {
		ERROR("Failed to load BL2 firmware.\n");
		plat_error_handler(err);
	}

	/* Allow platform to handle image information. */
	err = bl1_plat_handle_post_image_load(BL2_IMAGE_ID);
	if (err) {
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
#ifdef AARCH32
	NOTICE("BL1: Booting BL32\n");
#else
	NOTICE("BL1: Booting BL31\n");
#endif /* AARCH32 */
	print_entry_point_info(bl_ep_info);
}

#if SPIN_ON_BL1_EXIT
void print_debug_loop_message(void)
{
	NOTICE("BL1: Debug loop, spinning forever\n");
	NOTICE("BL1: Please connect the debugger to continue\n");
}
#endif

/*******************************************************************************
 * Top level handler for servicing BL1 SMCs.
 ******************************************************************************/
register_t bl1_smc_handler(unsigned int smc_fid,
	register_t x1,
	register_t x2,
	register_t x3,
	register_t x4,
	void *cookie,
	void *handle,
	unsigned int flags)
{

#if TRUSTED_BOARD_BOOT
	/*
	 * Dispatch FWU calls to FWU SMC handler and return its return
	 * value
	 */
	if (is_fwu_fid(smc_fid)) {
		return bl1_fwu_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
			handle, flags);
	}
#endif

	switch (smc_fid) {
	case BL1_SMC_CALL_COUNT:
		SMC_RET1(handle, BL1_NUM_SMC_CALLS);

	case BL1_SMC_UID:
		SMC_UUID_RET(handle, bl1_svc_uid);

	case BL1_SMC_VERSION:
		SMC_RET1(handle, BL1_SMC_MAJOR_VER | BL1_SMC_MINOR_VER);

	default:
		break;
	}

	WARN("Unimplemented BL1 SMC Call: 0x%x \n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

/*******************************************************************************
 * BL1 SMC wrapper.  This function is only used in AArch32 mode to ensure ABI
 * compliance when invoking bl1_smc_handler.
 ******************************************************************************/
register_t bl1_smc_wrapper(uint32_t smc_fid,
	void *cookie,
	void *handle,
	unsigned int flags)
{
	register_t x1, x2, x3, x4;

	assert(handle);

	get_smc_params_from_ctx(handle, x1, x2, x3, x4);
	return bl1_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
}
