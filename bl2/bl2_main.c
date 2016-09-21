/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <auth_mod.h>
#include <bl1.h>
#include <bl_common.h>
#include <debug.h>
#include <platform.h>
#include "bl2_private.h"


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

	/* Load the subsequent bootloader images. */
	next_bl_ep_info = bl2_load_images();

#ifdef AARCH32
	/*
	 * For AArch32 state BL1 and BL2 share the MMU setup.
	 * Given that BL2 does not map BL1 regions, MMU needs
	 * to be disabled in order to go back to BL1.
	 */
	disable_mmu_icache_secure();
#endif /* AARCH32 */

	/*
	 * Run next BL image via an SMC to BL1. Information on how to pass
	 * control to the BL32 (if present) and BL33 software images will
	 * be passed to next BL image as an argument.
	 */
	smc(BL1_SMC_RUN_IMAGE, (unsigned long)next_bl_ep_info, 0, 0, 0, 0, 0, 0);
}
