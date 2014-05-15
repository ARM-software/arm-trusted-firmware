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
#include <bl2.h>
#include <debug.h>
#include <platform.h>
#include <stdio.h>
#include "bl2_private.h"

/*******************************************************************************
 * Runs BL31 from the given entry point. It jumps to a higher exception level
 * through an SMC.
 ******************************************************************************/
static void __dead2 bl2_run_bl31(const el_change_info_t *bl31_ep_info)
{
	smc(RUN_IMAGE, (unsigned long)bl31_ep_info, 0, 0, 0, 0, 0, 0);
}


/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * BL31. The memory occupied by BL2 will be reclaimed by BL3_x stages. BL2 runs
 * entirely in S-EL1. Since arm standard c libraries are not PIC, printf et al
 * are not available. We rely on assertions to signal error conditions
 ******************************************************************************/
void bl2_main(void)
{
	meminfo_t *bl2_tzram_layout;
	bl31_args_t *bl2_to_bl31_args;
	unsigned long bl31_base, bl32_base = 0, bl33_base, el_status;
	unsigned int bl2_load, bl31_load, mode;

	/* Perform remaining generic architectural setup in S-El1 */
	bl2_arch_setup();

	/* Perform platform setup in BL1 */
	bl2_platform_setup();

	printf("BL2 %s\n\r", build_message);

	/*
	 * Get a pointer to the memory the platform has set aside to pass
	 * information to BL31.
	 */
	bl2_to_bl31_args = bl2_get_bl31_args_ptr();

	/* Find out how much free trusted ram remains after BL2 load */
	bl2_tzram_layout = bl2_plat_sec_mem_layout();

	/*
	 * Load BL31. BL1 tells BL2 whether it has been TOP or BOTTOM loaded.
	 * To avoid fragmentation of trusted SRAM memory, BL31 is always
	 * loaded opposite to BL2. This allows BL31 to reclaim BL2 memory
	 * while maintaining its free space in one contiguous chunk.
	 */
	bl2_load = bl2_tzram_layout->attr & LOAD_MASK;
	assert((bl2_load == TOP_LOAD) || (bl2_load == BOT_LOAD));
	bl31_load = (bl2_load == TOP_LOAD) ? BOT_LOAD : TOP_LOAD;
	bl31_base = load_image(bl2_tzram_layout, BL31_IMAGE_NAME,
	                       bl31_load, BL31_BASE);

	/* Assert if it has not been possible to load BL31 */
	if (bl31_base == 0) {
		ERROR("Failed to load BL3-1.\n");
		panic();
	}
	bl2_to_bl31_args->bl31_image_info.entrypoint = bl31_base;
	bl2_to_bl31_args->bl31_image_info.security_state = SECURE;
	bl2_to_bl31_args->bl31_image_info.spsr =
			SPSR_64(MODE_EL3, MODE_SP_ELX, DISABLE_ALL_EXCEPTION);

	/*
	 * Load the BL32 image if there's one. It is upto to platform
	 * to specify where BL32 should be loaded if it exists. It
	 * could create space in the secure sram or point to a
	 * completely different memory. A zero size indicates that the
	 * platform does not want to load a BL32 image.
	 */
	if (bl2_to_bl31_args->bl32_meminfo.total_size)
		bl32_base = load_image(&bl2_to_bl31_args->bl32_meminfo,
				       BL32_IMAGE_NAME,
				       bl2_to_bl31_args->bl32_meminfo.attr &
				       LOAD_MASK,
				       BL32_BASE);

	/*
	 * Create a new layout of memory for BL31 as seen by BL2. This
	 * will gobble up all the BL2 memory.
	 */
	init_bl31_mem_layout(bl2_tzram_layout,
			     &bl2_to_bl31_args->bl31_meminfo,
			     bl31_load);

	/* Load the BL33 image in non-secure memory provided by the platform */
	bl33_base = load_image(&bl2_to_bl31_args->bl33_meminfo,
			       BL33_IMAGE_NAME,
			       BOT_LOAD,
			       plat_get_ns_image_entrypoint());
	/* Halt if failed to load normal world firmware. */
	if (bl33_base == 0) {
		ERROR("Failed to load BL3-3.\n");
		panic();
	}

	/*
	 * BL2 also needs to tell BL31 where the non-trusted software image
	 * is located.
	 */
	bl2_to_bl31_args->bl33_image_info.entrypoint = bl33_base;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	if (el_status)
		mode = MODE_EL2;
	else
		mode = MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	bl2_to_bl31_args->bl33_image_info.spsr =
			SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTION);
	bl2_to_bl31_args->bl33_image_info.security_state = NON_SECURE;

	if (bl32_base) {
		/* Fill BL32 image info */
		bl2_to_bl31_args->bl32_image_info.entrypoint = bl32_base;
		bl2_to_bl31_args->bl32_image_info.security_state = SECURE;

		/*
		 * The Secure Payload Dispatcher service is responsible for
		 * setting the SPSR prior to entry into the BL32 image.
		 */
		bl2_to_bl31_args->bl32_image_info.spsr = 0;
	}

	/* Flush the entire BL31 args buffer */
	flush_dcache_range((unsigned long) bl2_to_bl31_args,
			   sizeof(*bl2_to_bl31_args));

	/* Set the args pointers for X0 and X1 to BL31 */
	bl2_to_bl31_args->bl31_image_info.args.arg0 =
				(unsigned long)bl2_to_bl31_args;
	bl2_to_bl31_args->bl31_image_info.args.arg1 = (unsigned long)0;

	/*
	 * Run BL31 via an SMC to BL1. Information on how to pass control to
	 * the BL32 (if present) and BL33 software images will be passed to
	 * BL31 as an argument.
	 */
	 bl2_run_bl31(&bl2_to_bl31_args->bl31_image_info);
}
