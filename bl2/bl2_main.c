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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <semihosting.h>
#include <bl_common.h>
#include <bl2.h>

/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * BL31. The memory occupied by BL2 will be reclaimed by BL3_x stages. BL2 runs
 * entirely in S-EL1. Since arm standard c libraries are not PIC, printf et al
 * are not available. We rely on assertions to signal error conditions
 ******************************************************************************/
void bl2_main(void)
{
	meminfo *bl2_tzram_layout;
	meminfo *bl31_tzram_layout;
	el_change_info *ns_image_info;
	unsigned long bl31_base, el_status;
	unsigned int bl2_load, bl31_load, mode;

	/* Perform remaining generic architectural setup in S-El1 */
	bl2_arch_setup();

	/* Perform platform setup in BL1 */
	bl2_platform_setup();

#if defined (__GNUC__)
	printf("BL2 Built : %s, %s\n\r", __TIME__, __DATE__);
#endif

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
	assert(bl31_base != 0);

	/*
	 * Create a new layout of memory for BL31 as seen by BL2. This
	 * will gobble up all the BL2 memory.
	 */
	bl31_tzram_layout = (meminfo *) get_el_change_mem_ptr();
	init_bl31_mem_layout(bl2_tzram_layout, bl31_tzram_layout, bl31_load);

	/*
	 * BL2 also needs to tell BL31 where the non-trusted software image
	 * has been loaded. Place this info right after the BL31 memory layout
	 */
	ns_image_info = (el_change_info *) ((unsigned char *) bl31_tzram_layout
					      + sizeof(meminfo));

	/*
	 * Assume that the non-secure bootloader has already been
	 * loaded to its platform-specific location.
	 */
	ns_image_info->entrypoint = plat_get_ns_image_entrypoint();

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	if (el_status)
		mode = MODE_EL2;
	else
		mode = MODE_EL1;

	ns_image_info->spsr = make_spsr(mode, MODE_SP_ELX, MODE_RW_64);
	ns_image_info->security_state = NON_SECURE;
	flush_dcache_range((unsigned long) ns_image_info,
			   sizeof(el_change_info));

	/*
	 * Run BL31 via an SMC to BL1. Information on how to pass control to
	 * the non-trusted software image will be passed to BL31 in x2.
	 */
	if (bl31_base)
		run_image(bl31_base,
			  make_spsr(MODE_EL3, MODE_SP_ELX, MODE_RW_64),
			  SECURE,
			  bl31_tzram_layout,
			  (void *) ns_image_info);

	/* There is no valid reason for run_image() to return */
	assert(0);
}
