/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#include <console.h>
#include <platform_def.h>
#include "qemu_private.h"


/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL1_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL1_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)


/*******************************************************************************
 * Declarations of linker defined symbols which will tell us where BL1 lives
 * in Trusted RAM
 ******************************************************************************/
extern uint64_t __BL1_RAM_START__;
extern uint64_t __BL1_RAM_END__;
#define BL1_RAM_BASE (uint64_t)(&__BL1_RAM_START__)
#define BL1_RAM_LIMIT (uint64_t)(&__BL1_RAM_END__)

/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo_t bl1_tzram_layout;


meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	const size_t bl1_size = BL1_RAM_LIMIT - BL1_RAM_BASE;

	/* Initialize the console to provide early debug support */
	console_init(PLAT_QEMU_BOOT_UART_BASE, PLAT_QEMU_BOOT_UART_CLK_IN_HZ,
		     PLAT_QEMU_CONSOLE_BAUDRATE);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL_RAM_BASE;
	bl1_tzram_layout.total_size = BL_RAM_SIZE;

	/* Calculate how much RAM BL1 is using and how much remains free */
	bl1_tzram_layout.free_base = BL_RAM_BASE;
	bl1_tzram_layout.free_size = BL_RAM_SIZE;
	reserve_mem(&bl1_tzram_layout.free_base, &bl1_tzram_layout.free_size,
		    BL1_RAM_BASE, bl1_size);
}

/******************************************************************************
 * Perform the very early platform specific architecture setup.  This only
 * does basic initialization. Later architectural setup (bl1_arch_setup())
 * does not do anything platform specific.
 *****************************************************************************/
void bl1_plat_arch_setup(void)
{
	qemu_configure_mmu_el3(bl1_tzram_layout.total_base,
				bl1_tzram_layout.total_size,
				BL1_RO_BASE, BL1_RO_LIMIT,
				BL1_COHERENT_RAM_BASE, BL1_COHERENT_RAM_LIMIT);
}

void bl1_platform_setup(void)
{
	plat_qemu_io_setup();
}

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
	       (BL1_RAM_BASE >= (bl1_mem_layout->free_base +
				 bl1_mem_layout->free_size)));

	/* Remove BL1 RW data from the scope of memory visible to BL2 */
	*bl2_mem_layout = *bl1_mem_layout;
	reserve_mem(&bl2_mem_layout->total_base,
		    &bl2_mem_layout->total_size,
		    BL1_RAM_BASE,
		    bl1_size);

	flush_dcache_range((unsigned long)bl2_mem_layout, sizeof(meminfo_t));
}

/*******************************************************************************
 * Before calling this function BL2 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL2 and set SPSR and security state.
 * On ARM standard platforms we only set the security state of the entrypoint
 ******************************************************************************/
void bl1_plat_set_bl2_ep_info(image_info_t *bl2_image,
				entry_point_info_t *bl2_ep)
{
	SET_SECURITY_STATE(bl2_ep->h.attr, SECURE);
	bl2_ep->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
}
