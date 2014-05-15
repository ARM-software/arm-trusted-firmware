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

#include <bl_common.h>
#include <console.h>
#include <fvp_def.h>
#include "fvp_private.h"

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
#define BL32_RO_BASE (unsigned long)(&__RO_START__)
#define BL32_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL32_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL32_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

/* Data structure which holds the extents of the trusted SRAM for BL32 */
static meminfo_t bl32_tzdram_layout
__attribute__ ((aligned(PLATFORM_CACHE_LINE_SIZE),
		section("tzfw_coherent_mem")));

meminfo_t *bl32_plat_sec_mem_layout(void)
{
	return &bl32_tzdram_layout;
}

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that's at BL32's disposal.
 * Initialize the BL32 data structure with the memory extends and initialize
 * the UART
 ******************************************************************************/
void bl32_early_platform_setup(meminfo_t *mem_layout,
			      void *data)
{
	/*
	 * Initialize a different console than already in use to display
	 * messages from TSP
	 */
	console_init(PL011_UART1_BASE);

	/* Setup the BL32 memory layout */
	bl32_tzdram_layout.total_base = mem_layout->total_base;
	bl32_tzdram_layout.total_size = mem_layout->total_size;
	bl32_tzdram_layout.free_base = mem_layout->free_base;
	bl32_tzdram_layout.free_size = mem_layout->free_size;
	bl32_tzdram_layout.attr = mem_layout->attr;
	bl32_tzdram_layout.next = 0;

}

/*******************************************************************************
 * Perform platform specific setup placeholder
 ******************************************************************************/
void bl32_platform_setup()
{

}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the MMU
 ******************************************************************************/
void bl32_plat_arch_setup()
{
	fvp_configure_mmu_el1(&bl32_tzdram_layout,
			  BL32_RO_BASE,
			  BL32_RO_LIMIT,
			  BL32_COHERENT_RAM_BASE,
			  BL32_COHERENT_RAM_LIMIT);
}
