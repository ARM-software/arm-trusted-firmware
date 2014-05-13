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
#include <bl1.h>
#include <platform.h>
#include <stdio.h>
#include "bl1_private.h"

/*******************************************************************************
 * Function to perform late architectural and platform specific initialization.
 * It also locates and loads the BL2 raw binary image in the trusted DRAM. Only
 * called by the primary cpu after a cold boot.
 * TODO: Add support for alternative image load mechanism e.g using virtio/elf
 * loader etc.
  ******************************************************************************/
void bl1_main(void)
{
#if DEBUG
	unsigned long sctlr_el3 = read_sctlr_el3();
#endif
	unsigned long bl2_base;
	unsigned int load_type = TOP_LOAD, spsr;
	meminfo_t *bl1_tzram_layout;
	meminfo_t *bl2_tzram_layout = 0x0;

	/*
	 * Ensure that MMU/Caches and coherency are turned on
	 */
	assert(sctlr_el3 | SCTLR_M_BIT);
	assert(sctlr_el3 | SCTLR_C_BIT);
	assert(sctlr_el3 | SCTLR_I_BIT);

	/* Perform remaining generic architectural setup from EL3 */
	bl1_arch_setup();

	/* Perform platform setup in BL1. */
	bl1_platform_setup();

	/* Announce our arrival */
	printf(FIRMWARE_WELCOME_STR);
	printf("%s\n\r", build_message);

	/*
	 * Find out how much free trusted ram remains after BL1 load
	 * & load the BL2 image at its top
	 */
	bl1_tzram_layout = bl1_plat_sec_mem_layout();
	bl2_base = load_image(bl1_tzram_layout,
			      (const char *) BL2_IMAGE_NAME,
			      load_type, BL2_BASE);

	/*
	 * Create a new layout of memory for BL2 as seen by BL1 i.e.
	 * tell it the amount of total and free memory available.
	 * This layout is created at the first free address visible
	 * to BL2. BL2 will read the memory layout before using its
	 * memory for other purposes.
	 */
	bl2_tzram_layout = (meminfo_t *) bl1_tzram_layout->free_base;
	init_bl2_mem_layout(bl1_tzram_layout,
			    bl2_tzram_layout,
			    load_type,
			    bl2_base);

	if (bl2_base) {
		bl1_arch_next_el_setup();
		spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTION);
		printf("Booting trusted firmware boot loader stage 2\n\r");
#if DEBUG
		printf("BL2 address = 0x%llx \n\r", (unsigned long long) bl2_base);
		printf("BL2 cpsr = 0x%x \n\r", spsr);
		printf("BL2 memory layout address = 0x%llx \n\r",
		       (unsigned long long) bl2_tzram_layout);
#endif
		run_image(bl2_base,
			  spsr,
			  SECURE,
			  (void *) bl2_tzram_layout,
			  NULL);
	}

	/*
	 * TODO: print failure to load BL2 but also add a tzwdog timer
	 * which will reset the system eventually.
	 */
	printf("Failed to load boot loader stage 2 (BL2) firmware.\n\r");
	return;
}

/*******************************************************************************
 * Temporary function to print the fact that BL2 has done its job and BL31 is
 * about to be loaded. This is needed as long as printfs cannot be used
 ******************************************************************************/
void display_boot_progress(unsigned long entrypoint,
			   unsigned long spsr,
			   unsigned long mem_layout,
			   unsigned long ns_image_info)
{
	printf("Booting trusted firmware boot loader stage 3\n\r");
#if DEBUG
	printf("BL31 address = 0x%llx \n\r", (unsigned long long) entrypoint);
	printf("BL31 cpsr = 0x%llx \n\r", (unsigned long long)spsr);
	printf("BL31 memory layout address = 0x%llx \n\r", (unsigned long long)mem_layout);
	printf("BL31 non-trusted image info address = 0x%llx\n\r", (unsigned long long)ns_image_info);
#endif
	return;
}
