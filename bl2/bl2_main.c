/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <bl1/bl1.h>
#include <bl2/bl2.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/console.h>
#include <plat/common/platform.h>

#include "bl2_private.h"

#ifdef AARCH32
#define NEXT_IMAGE	"BL32"
#else
#define NEXT_IMAGE	"BL31"
#endif

#if !BL2_AT_EL3
/*******************************************************************************
 * Setup function for BL2.
 ******************************************************************************/
void bl2_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
	       u_register_t arg3)
{
	/* Perform early platform-specific setup */
	bl2_early_platform_setup2(arg0, arg1, arg2, arg3);

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
	bl2_plat_arch_setup();
}

#else /* if BL2_AT_EL3 */
/*******************************************************************************
 * Setup function for BL2 when BL2_AT_EL3=1.
 ******************************************************************************/
void bl2_el3_setup(u_register_t arg0, u_register_t arg1, u_register_t arg2,
		   u_register_t arg3)
{
	/* Perform early platform-specific setup */
	bl2_el3_early_platform_setup(arg0, arg1, arg2, arg3);

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
	bl2_el3_plat_arch_setup();
}
#endif /* BL2_AT_EL3 */

/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * next BL. The memory occupied by BL2 will be reclaimed by BL3x stages. BL2
 * runs entirely in S-EL1.
 ******************************************************************************/
void bl2_main(void)
{
	entry_point_info_t *next_bl_ep_info;

	NOTICE("BL2: %s\n", version_string);
	NOTICE("BL2: %s\n", build_message);

	/* Perform remaining generic architectural setup in S-EL1 */
	bl2_arch_setup();

#if TRUSTED_BOARD_BOOT
	/* Initialize authentication module */
	auth_mod_init();
#endif /* TRUSTED_BOARD_BOOT */

	/* initialize boot source */
	bl2_plat_preload_setup();

	/* Load the subsequent bootloader images. */
	next_bl_ep_info = bl2_load_images();

#if !BL2_AT_EL3
#ifdef AARCH32
	/*
	 * For AArch32 state BL1 and BL2 share the MMU setup.
	 * Given that BL2 does not map BL1 regions, MMU needs
	 * to be disabled in order to go back to BL1.
	 */
	disable_mmu_icache_secure();
#endif /* AARCH32 */

	console_flush();

	/*
	 * Run next BL image via an SMC to BL1. Information on how to pass
	 * control to the BL32 (if present) and BL33 software images will
	 * be passed to next BL image as an argument.
	 */
	smc(BL1_SMC_RUN_IMAGE, (unsigned long)next_bl_ep_info, 0, 0, 0, 0, 0, 0);
#else /* if BL2_AT_EL3 */
	NOTICE("BL2: Booting " NEXT_IMAGE "\n");
	print_entry_point_info(next_bl_ep_info);
	console_flush();

	bl2_run_next_image(next_bl_ep_info);
#endif /* BL2_AT_EL3 */
}
