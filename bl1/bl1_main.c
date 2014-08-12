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
#include "bl1_private.h"

/*******************************************************************************
 * Runs BL2 from the given entry point. It results in dropping the
 * exception level
 ******************************************************************************/
static void __dead2 bl1_run_bl2(entry_point_info_t *bl2_ep)
{
	bl1_arch_next_el_setup();

	/* Tell next EL what we want done */
	bl2_ep->args.arg0 = RUN_IMAGE;

	if (GET_SECURITY_STATE(bl2_ep->h.attr) == NON_SECURE)
		change_security_state(GET_SECURITY_STATE(bl2_ep->h.attr));

	write_spsr_el3(bl2_ep->spsr);
	write_elr_el3(bl2_ep->pc);

	eret(bl2_ep->args.arg0,
		bl2_ep->args.arg1,
		bl2_ep->args.arg2,
		bl2_ep->args.arg3,
		bl2_ep->args.arg4,
		bl2_ep->args.arg5,
		bl2_ep->args.arg6,
		bl2_ep->args.arg7);
}

/*******************************************************************************
 * The next function has a weak definition. Platform specific code can override
 * it if it wishes to.
 ******************************************************************************/
#pragma weak bl1_init_bl2_mem_layout

/*******************************************************************************
 * Function that takes a memory layout into which BL2 has been loaded and
 * populates a new memory layout for BL2 that ensures that BL1's data sections
 * resident in secure RAM are not visible to BL2.
 ******************************************************************************/
void bl1_init_bl2_mem_layout(const meminfo_t *bl1_mem_layout,
			     meminfo_t *bl2_mem_layout)
{
	const size_t bl1_size = BL1_RAM_LIMIT - BL1_RAM_BASE;

	assert(bl1_mem_layout != NULL);
	assert(bl2_mem_layout != NULL);

	/* Check that BL1's memory is lying outside of the free memory */
	assert((BL1_RAM_LIMIT <= bl1_mem_layout->free_base) ||
	       (BL1_RAM_BASE >= bl1_mem_layout->free_base + bl1_mem_layout->free_size));

	/* Remove BL1 RW data from the scope of memory visible to BL2 */
	*bl2_mem_layout = *bl1_mem_layout;
	reserve_mem(&bl2_mem_layout->total_base,
		    &bl2_mem_layout->total_size,
		    BL1_RAM_BASE,
		    bl1_size);

	flush_dcache_range((unsigned long)bl2_mem_layout, sizeof(meminfo_t));
}

/*******************************************************************************
 * Function to perform late architectural and platform specific initialization.
 * It also locates and loads the BL2 raw binary image in the trusted DRAM. Only
 * called by the primary cpu after a cold boot.
 * TODO: Add support for alternative image load mechanism e.g using virtio/elf
 * loader etc.
  ******************************************************************************/
void bl1_main(void)
{
	/* Announce our arrival */
	NOTICE(FIRMWARE_WELCOME_STR);
	NOTICE("BL1: %s\n", version_string);
	NOTICE("BL1: %s\n", build_message);

	INFO("BL1: RAM 0x%lx - 0x%lx\n", BL1_RAM_BASE, BL1_RAM_LIMIT);

#if DEBUG
	unsigned long sctlr_el3 = read_sctlr_el3();
#endif
	image_info_t bl2_image_info = { {0} };
	entry_point_info_t bl2_ep = { {0} };
	meminfo_t *bl1_tzram_layout;
	meminfo_t *bl2_tzram_layout = 0x0;
	int err;

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

	SET_PARAM_HEAD(&bl2_image_info, PARAM_IMAGE_BINARY, VERSION_1, 0);
	SET_PARAM_HEAD(&bl2_ep, PARAM_EP, VERSION_1, 0);

	/* Find out how much free trusted ram remains after BL1 load */
	bl1_tzram_layout = bl1_plat_sec_mem_layout();

	/* Load the BL2 image */
	err = load_image(bl1_tzram_layout,
			 BL2_IMAGE_NAME,
			 BL2_BASE,
			 &bl2_image_info,
			 &bl2_ep);
	if (err) {
		/*
		 * TODO: print failure to load BL2 but also add a tzwdog timer
		 * which will reset the system eventually.
		 */
		ERROR("Failed to load BL2 firmware.\n");
		panic();
	}
	/*
	 * Create a new layout of memory for BL2 as seen by BL1 i.e.
	 * tell it the amount of total and free memory available.
	 * This layout is created at the first free address visible
	 * to BL2. BL2 will read the memory layout before using its
	 * memory for other purposes.
	 */
	bl2_tzram_layout = (meminfo_t *) bl1_tzram_layout->free_base;
	bl1_init_bl2_mem_layout(bl1_tzram_layout, bl2_tzram_layout);

	bl1_plat_set_bl2_ep_info(&bl2_image_info, &bl2_ep);
	bl2_ep.args.arg1 = (unsigned long)bl2_tzram_layout;
	NOTICE("BL1: Booting BL2\n");
	INFO("BL1: BL2 address = 0x%llx\n",
		(unsigned long long) bl2_ep.pc);
	INFO("BL1: BL2 spsr = 0x%x\n", bl2_ep.spsr);
	VERBOSE("BL1: BL2 memory layout address = 0x%llx\n",
		(unsigned long long) bl2_tzram_layout);

	bl1_run_bl2(&bl2_ep);

	return;
}

/*******************************************************************************
 * Temporary function to print the fact that BL2 has done its job and BL31 is
 * about to be loaded. This is needed as long as printfs cannot be used
 ******************************************************************************/
void display_boot_progress(entry_point_info_t *bl31_ep_info)
{
	NOTICE("BL1: Booting BL3-1\n");
	INFO("BL1: BL3-1 address = 0x%llx\n",
		(unsigned long long)bl31_ep_info->pc);
	INFO("BL1: BL3-1 spsr = 0x%llx\n",
		(unsigned long long)bl31_ep_info->spsr);
	INFO("BL1: BL3-1 params address = 0x%llx\n",
		(unsigned long long)bl31_ep_info->args.arg0);
	INFO("BL1: BL3-1 plat params address = 0x%llx\n",
		(unsigned long long)bl31_ep_info->args.arg1);
}
