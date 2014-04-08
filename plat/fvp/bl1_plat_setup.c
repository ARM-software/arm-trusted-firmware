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
#include <bl1.h>
#include <console.h>
#include <cci400.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

extern unsigned long __BL1_RAM_START__;
extern unsigned long __BL1_RAM_END__;

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL1_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL1_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

#define BL1_RAM_BASE (unsigned long)(&__BL1_RAM_START__)
#define BL1_RAM_LIMIT (unsigned long)(&__BL1_RAM_END__)


/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo bl1_tzram_layout;

meminfo *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	const unsigned long bl1_ram_base = BL1_RAM_BASE;
	const unsigned long bl1_ram_limit = BL1_RAM_LIMIT;
	const unsigned long tzram_limit = TZRAM_BASE + TZRAM_SIZE;

	/* Initialize the console to provide early debug support */
	console_init(PL011_UART0_BASE);

	/*
	 * Calculate how much ram is BL1 using & how much remains free.
	 * This also includes a rudimentary mechanism to detect whether
	 * the BL1 data is loaded at the top or bottom of memory.
	 * TODO: add support for discontigous chunks of free ram if
	 *       needed. Might need dynamic memory allocation support
	 *       et al.
	 */
	bl1_tzram_layout.total_base = TZRAM_BASE;
	bl1_tzram_layout.total_size = TZRAM_SIZE;

	if (bl1_ram_limit == tzram_limit) {
		/* BL1 has been loaded at the top of memory. */
		bl1_tzram_layout.free_base = TZRAM_BASE;
		bl1_tzram_layout.free_size = bl1_ram_base - TZRAM_BASE;
	} else {
		/* BL1 has been loaded at the bottom of memory. */
		bl1_tzram_layout.free_base = bl1_ram_limit;
		bl1_tzram_layout.free_size =
			tzram_limit - bl1_ram_limit;
	}

	/* Initialize the platform config for future decision making */
	platform_config_setup();
}

/*******************************************************************************
 * Function which will evaluate how much of the trusted ram has been gobbled
 * up by BL1 and return the base and size of whats available for loading BL2.
 * Its called after coherency and the MMU have been turned on.
 ******************************************************************************/
void bl1_platform_setup(void)
{
	unsigned int counter_base_frequency;

	/* Initialise the IO layer and register platform IO devices */
	io_setup();

	/*
	 * Enable and initialize the System level generic timer. Choose base
	 * frequency for the timer
	 */
	mmio_write_32(SYS_CNTCTL_BASE + CNTCR_OFF, CNTCR_FCREQ(0) | CNTCR_EN);

	/* Read the frequency from Frequency modes table */
	counter_base_frequency = mmio_read_32(SYS_CNTCTL_BASE + CNTFID_OFF);

	/* The first entry of the frequency modes table must not be 0 */
	assert(counter_base_frequency != 0);

	/* Program the counter frequency */
	write_cntfrq_el0(counter_base_frequency);
}


/*******************************************************************************
 * Perform the very early platform specific architecture setup here. At the
 * moment this only does basic initialization. Later architectural setup
 * (bl1_arch_setup()) does not do anything platform specific.
 ******************************************************************************/
void bl1_plat_arch_setup(void)
{
	unsigned long cci_setup;

	/*
	 * Enable CCI-400 for this cluster. No need
	 * for locks as no other cpu is active at the
	 * moment
	 */
	cci_setup = platform_get_cfgvar(CONFIG_HAS_CCI);
	if (cci_setup) {
		cci_enable_coherency(read_mpidr());
	}

	configure_mmu(&bl1_tzram_layout,
			TZROM_BASE,
			TZROM_BASE + TZROM_SIZE,
			BL1_COHERENT_RAM_BASE,
			BL1_COHERENT_RAM_LIMIT);
}
