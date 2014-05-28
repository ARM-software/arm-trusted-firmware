/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include <stdio.h>
#include "bl2_private.h"


/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * BL31. The memory occupied by BL2 will be reclaimed by BL3_x stages. BL2 runs
 * entirely in S-EL1. Since arm standard c libraries are not PIC, printf et al
 * are not available. We rely on assertions to signal error conditions
 ******************************************************************************/
void bl2_main(void)
{
	meminfo_t *bl2_tzram_layout;
	bl31_params_t *bl2_to_bl31_params;
	unsigned int bl2_load, bl31_load;
	entry_point_info_t *bl31_ep_info;
	meminfo_t bl32_mem_info;
	meminfo_t bl33_mem_info;
	int e;

	/* Perform remaining generic architectural setup in S-El1 */
	bl2_arch_setup();

	/* Perform platform setup in BL1 */
	bl2_platform_setup();

	printf("BL2 %s\n\r", build_message);

	/* Find out how much free trusted ram remains after BL2 load */
	bl2_tzram_layout = bl2_plat_sec_mem_layout();

	/*
	 * Get a pointer to the memory the platform has set aside to pass
	 * information to BL31.
	 */
	bl2_to_bl31_params = bl2_plat_get_bl31_params();
	bl31_ep_info = bl2_plat_get_bl31_ep_info();

	/* Set the X0 parameter to bl31 */
	bl31_ep_info->args.arg0 = (unsigned long)bl2_to_bl31_params;

	/*
	 * Load BL31. BL1 tells BL2 whether it has been TOP or BOTTOM loaded.
	 * To avoid fragmentation of trusted SRAM memory, BL31 is always
	 * loaded opposite to BL2. This allows BL31 to reclaim BL2 memory
	 * while maintaining its free space in one contiguous chunk.
	 */
	bl2_load = bl2_tzram_layout->attr & LOAD_MASK;
	assert((bl2_load == TOP_LOAD) || (bl2_load == BOT_LOAD));
	bl31_load = (bl2_load == TOP_LOAD) ? BOT_LOAD : TOP_LOAD;
	e = load_image(bl2_tzram_layout,
			BL31_IMAGE_NAME,
			bl31_load,
			BL31_BASE,
			bl2_to_bl31_params->bl31_image_info,
			bl31_ep_info);

	/* Assert if it has not been possible to load BL31 */
	if (e) {
		ERROR("Failed to load BL3-1.\n");
		panic();
	}

	bl2_plat_set_bl31_ep_info(bl2_to_bl31_params->bl31_image_info,
				bl31_ep_info);

	bl2_plat_get_bl33_meminfo(&bl33_mem_info);

	/* Load the BL33 image in non-secure memory provided by the platform */
	e = load_image(&bl33_mem_info,
			BL33_IMAGE_NAME,
			BOT_LOAD,
			plat_get_ns_image_entrypoint(),
			bl2_to_bl31_params->bl33_image_info,
			bl2_to_bl31_params->bl33_ep_info);

	/* Halt if failed to load normal world firmware. */
	if (e) {
		ERROR("Failed to load BL3-3.\n");
		panic();
	}
	bl2_plat_set_bl33_ep_info(bl2_to_bl31_params->bl33_image_info,
				bl2_to_bl31_params->bl33_ep_info);


#ifdef BL32_BASE
	/*
	 * Load the BL32 image if there's one. It is upto to platform
	 * to specify where BL32 should be loaded if it exists. It
	 * could create space in the secure sram or point to a
	 * completely different memory.
	 *
	 * If a platform does not want to attempt to load BL3-2 image
	 * it must leave BL32_BASE undefined
	 */
	bl2_plat_get_bl32_meminfo(&bl32_mem_info);
	e = load_image(&bl32_mem_info,
		       BL32_IMAGE_NAME,
		       bl32_mem_info.attr & LOAD_MASK,
		       BL32_BASE,
		       bl2_to_bl31_params->bl32_image_info,
		       bl2_to_bl31_params->bl32_ep_info);

	/* Issue a diagnostic if no Secure Payload could be loaded */
	if (e) {
		WARN("Failed to load BL3-2.\n");
	} else {
		bl2_plat_set_bl32_ep_info(
			bl2_to_bl31_params->bl32_image_info,
			bl2_to_bl31_params->bl32_ep_info);
	}
#endif /* BL32_BASE */

	/* Flush the params to be passed to memory */
	bl2_plat_flush_bl31_params();

	/*
	 * Run BL31 via an SMC to BL1. Information on how to pass control to
	 * the BL32 (if present) and BL33 software images will be passed to
	 * BL31 as an argument.
	 */
	smc(RUN_IMAGE, (unsigned long)bl31_ep_info, 0, 0, 0, 0, 0, 0);
}
