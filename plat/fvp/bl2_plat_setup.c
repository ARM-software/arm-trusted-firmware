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

#include <assert.h>
#include <arch_helpers.h>
#include <platform.h>
#include <bl2.h>
#include <bl_common.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
extern unsigned long __RO_START__;
extern unsigned long __RO_END__;

extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL2_RO_BASE (unsigned long)(&__RO_START__)
#define BL2_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL2_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL2_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

/* Pointer to memory visible to both BL2 and BL31 for passing data */
extern unsigned char **bl2_el_change_mem_ptr;

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo bl2_tzram_layout
__attribute__ ((aligned(PLATFORM_CACHE_LINE_SIZE),
		section("tzfw_coherent_mem")));

/*******************************************************************************
 * Reference to structure which holds the arguments which need to be passed
 * to BL31
 ******************************************************************************/
static bl31_args *bl2_to_bl31_args;

meminfo *bl2_plat_sec_mem_layout(void)
{
	return &bl2_tzram_layout;
}

/*******************************************************************************
 * This function returns a pointer to the memory that the platform has kept
 * aside to pass all the information that BL31 could need.
 ******************************************************************************/
bl31_args *bl2_get_bl31_args_ptr(void)
{
	return bl2_to_bl31_args;
}

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe loaction before its reclaimed by later BL2 functionality.
 ******************************************************************************/
void bl2_early_platform_setup(meminfo *mem_layout,
			      void *data)
{
	/* Setup the BL2 memory layout */
	bl2_tzram_layout.total_base = mem_layout->total_base;
	bl2_tzram_layout.total_size = mem_layout->total_size;
	bl2_tzram_layout.free_base = mem_layout->free_base;
	bl2_tzram_layout.free_size = mem_layout->free_size;
	bl2_tzram_layout.attr = mem_layout->attr;
	bl2_tzram_layout.next = 0;

	/* Initialize the platform config for future decision making */
	platform_config_setup();

	return;
}

/*******************************************************************************
 * Perform platform specific setup. For now just initialize the memory location
 * to use for passing arguments to BL31.
 ******************************************************************************/
void bl2_platform_setup()
{
	/* Initialise the IO layer and register platform IO devices */
	io_setup();

	/*
	 * Ensure that the secure DRAM memory used for passing BL31 arguments
	 * does not overlap with the BL32_BASE.
	 */
	assert (BL32_BASE > TZDRAM_BASE + sizeof(bl31_args));

	/* Use the Trusted DRAM for passing args to BL31 */
	bl2_to_bl31_args = (bl31_args *) TZDRAM_BASE;

	/* Populate the extents of memory available for loading BL33 */
	bl2_to_bl31_args->bl33_meminfo.total_base = DRAM_BASE;
	bl2_to_bl31_args->bl33_meminfo.total_size = DRAM_SIZE;
	bl2_to_bl31_args->bl33_meminfo.free_base = DRAM_BASE;
	bl2_to_bl31_args->bl33_meminfo.free_size = DRAM_SIZE;
	bl2_to_bl31_args->bl33_meminfo.attr = 0;
	bl2_to_bl31_args->bl33_meminfo.next = 0;

	/*
	 * Populate the extents of memory available for loading BL32.
	 * TODO: We are temporarily executing BL2 from TZDRAM; will eventually
	 * move to Trusted SRAM
	 */
	bl2_to_bl31_args->bl32_meminfo.total_base = BL32_BASE;
	bl2_to_bl31_args->bl32_meminfo.free_base = BL32_BASE;

	bl2_to_bl31_args->bl32_meminfo.total_size =
		(TZDRAM_BASE + TZDRAM_SIZE) - BL32_BASE;
	bl2_to_bl31_args->bl32_meminfo.free_size =
		(TZDRAM_BASE + TZDRAM_SIZE) - BL32_BASE;

	bl2_to_bl31_args->bl32_meminfo.attr = BOT_LOAD;
	bl2_to_bl31_args->bl32_meminfo.next = 0;
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl2_plat_arch_setup()
{
	configure_mmu(&bl2_tzram_layout,
		      BL2_RO_BASE,
		      BL2_RO_LIMIT,
		      BL2_COHERENT_RAM_BASE,
		      BL2_COHERENT_RAM_LIMIT);
}
