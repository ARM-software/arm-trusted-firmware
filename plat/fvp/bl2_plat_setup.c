/*
 * Copyright (c) 2013, ARM Limited. All rights reserved.
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

#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <platform.h>
#include <bl2.h>
#include <bl_common.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
#if defined (__GNUC__)
extern unsigned long __BL2_RO_BASE__;
extern unsigned long __BL2_STACKS_BASE__;
extern unsigned long __BL2_COHERENT_RAM_BASE__;
extern unsigned long __BL2_RW_BASE__;

#define BL2_RO_BASE		__BL2_RO_BASE__
#define BL2_STACKS_BASE		__BL2_STACKS_BASE__
#define BL2_COHERENT_RAM_BASE	__BL2_COHERENT_RAM_BASE__
#define BL2_RW_BASE		__BL2_RW_BASE__

#else
 #error "Unknown compiler."
#endif

/* Pointer to memory visible to both BL2 and BL31 for passing data */
extern unsigned char **bl2_el_change_mem_ptr;

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo bl2_tzram_layout
__attribute__ ((aligned(PLATFORM_CACHE_LINE_SIZE),
		section("tzfw_coherent_mem")));

/* Data structure which holds the extents of the non-trusted DRAM for BL2*/
static meminfo dram_layout;

meminfo bl2_get_sec_mem_layout(void)
{
	return bl2_tzram_layout;
}

meminfo bl2_get_ns_mem_layout(void)
{
	return dram_layout;
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
 * Not much to do here aprt from finding out the extents of non-trusted DRAM
 * which will be used for loading the non-trusted software images. We are
 * relying on pre-iniitialized zi memory so there is nothing to zero out like
 * in BL1. This is 'cause BL2 is raw PIC binary. Its load address is determined
 * at runtime. The ZI section might be lost if its not already there.
 ******************************************************************************/
void bl2_platform_setup()
{
	dram_layout.total_base = DRAM_BASE;
	dram_layout.total_size = DRAM_SIZE;
	dram_layout.free_base = DRAM_BASE;
	dram_layout.free_size = DRAM_SIZE;
	dram_layout.attr = 0;

	/* Use the Trusted DRAM for passing args to BL31 */
	bl2_el_change_mem_ptr = (unsigned char **) TZDRAM_BASE;

	return;
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl2_plat_arch_setup()
{
	configure_mmu(&bl2_tzram_layout,
		      (unsigned long) &BL2_RO_BASE,
		      (unsigned long) &BL2_STACKS_BASE,
		      (unsigned long) &BL2_COHERENT_RAM_BASE,
		      (unsigned long) &BL2_RW_BASE);
}
