/*
 * Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/console.h>
#include <lib/bootmarker_capture.h>
#include <lib/cpus/errata.h>
#include <lib/pmf/pmf.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <smccc_helpers.h>
#include <tools_share/uuid.h>

#include "bl1_private.h"

static void bl1_load_bl2(void);

#if ENABLE_PAUTH
uint64_t bl1_apiakey[2];
#endif

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_REGISTER_SERVICE(bl_svc, PMF_RT_INSTR_SVC_ID,
		BL_TOTAL_IDS, PMF_DUMP_ENABLE)
#endif

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

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_CAPTURE_TIMESTAMP(bl_svc, BL1_ENTRY, PMF_CACHE_MAINT);
#endif

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
#ifdef __aarch64__
	val = read_sctlr_el3();
#else
	val = read_sctlr();
#endif
	assert((val & SCTLR_M_BIT) != 0);
	assert((val & SCTLR_C_BIT) != 0);
	assert((val & SCTLR_I_BIT) != 0);
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

	crypto_mod_init();

	/* Initialize authentication module */
	auth_mod_init();

	/* Initialize the measured boot */
	bl1_plat_mboot_init();

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
	if (image_id == BL2_IMAGE_ID)
		bl1_load_bl2();
	else
		NOTICE("BL1-FWU: *******FWU Process Started*******\n");

	/* Teardown the measured boot driver */
	bl1_plat_mboot_finish();

	bl1_prepare_next_image(image_id);

#if ENABLE_RUNTIME_INSTRUMENTATION
	PMF_CAPTURE_TIMESTAMP(bl_svc, BL1_EXIT, PMF_CACHE_MAINT);
#endif

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
#ifdef __aarch64__
	NOTICE("BL1: Booting BL31\n");
#else
	NOTICE("BL1: Booting BL32\n");
#endif /* __aarch64__ */
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
u_register_t bl1_smc_handler(unsigned int smc_fid,
	u_register_t x1,
	u_register_t x2,
	u_register_t x3,
	u_register_t x4,
	void *cookie,
	void *handle,
	unsigned int flags)
{
	/* BL1 Service UUID */
	DEFINE_SVC_UUID2(bl1_svc_uid,
		U(0xd46739fd), 0xcb72, 0x9a4d, 0xb5, 0x75,
		0x67, 0x15, 0xd6, 0xf4, 0xbb, 0x4a);


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
		WARN("Unimplemented BL1 SMC Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/*******************************************************************************
 * BL1 SMC wrapper.  This function is only used in AArch32 mode to ensure ABI
 * compliance when invoking bl1_smc_handler.
 ******************************************************************************/
u_register_t bl1_smc_wrapper(uint32_t smc_fid,
	void *cookie,
	void *handle,
	unsigned int flags)
{
	u_register_t x1, x2, x3, x4;

	assert(handle != NULL);

	get_smc_params_from_ctx(handle, x1, x2, x3, x4);
	return bl1_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
}
