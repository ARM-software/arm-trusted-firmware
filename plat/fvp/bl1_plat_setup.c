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
#include <bl1.h>
#include <console.h>
#include <cci400.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
#if defined (__GNUC__)
extern unsigned long __FIRMWARE_ROM_START__;
extern unsigned long __FIRMWARE_ROM_SIZE__;
extern unsigned long __FIRMWARE_DATA_START__;
extern unsigned long __FIRMWARE_DATA_SIZE__;
extern unsigned long __FIRMWARE_BSS_START__;
extern unsigned long __FIRMWARE_BSS_SIZE__;
extern unsigned long __DATA_RAM_START__;
extern unsigned long __DATA_RAM_SIZE__;
extern unsigned long __BSS_RAM_START__;
extern unsigned long __BSS_RAM_SIZE__;
extern unsigned long __FIRMWARE_RAM_STACKS_START__;
extern unsigned long __FIRMWARE_RAM_STACKS_SIZE__;
extern unsigned long __FIRMWARE_RAM_PAGETABLES_START__;
extern unsigned long __FIRMWARE_RAM_PAGETABLES_SIZE__;
extern unsigned long __FIRMWARE_RAM_COHERENT_START__;
extern unsigned long __FIRMWARE_RAM_COHERENT_SIZE__;

#define BL1_COHERENT_MEM_BASE	(&__FIRMWARE_RAM_COHERENT_START__)
#define BL1_COHERENT_MEM_LIMIT \
	((unsigned long long)&__FIRMWARE_RAM_COHERENT_START__ + \
	 (unsigned long long)&__FIRMWARE_RAM_COHERENT_SIZE__)

#define BL1_FIRMWARE_RAM_GLOBALS_ZI_BASE \
	(unsigned long)(&__BSS_RAM_START__)
#define BL1_FIRMWARE_RAM_GLOBALS_ZI_LENGTH \
	(unsigned long)(&__FIRMWARE_BSS_SIZE__)

#define BL1_FIRMWARE_RAM_COHERENT_ZI_BASE \
	(unsigned long)(&__FIRMWARE_RAM_COHERENT_START__)
#define BL1_FIRMWARE_RAM_COHERENT_ZI_LENGTH\
	(unsigned long)(&__FIRMWARE_RAM_COHERENT_SIZE__)

#define BL1_NORMAL_RAM_BASE (unsigned long)(&__BSS_RAM_START__)
#define BL1_NORMAL_RAM_LIMIT \
	((unsigned long)&__FIRMWARE_RAM_COHERENT_START__ +	\
	 (unsigned long)&__FIRMWARE_RAM_COHERENT_SIZE__)
#else
 #error "Unknown compiler."
#endif


/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo bl1_tzram_layout;

meminfo bl1_get_sec_mem_layout(void)
{
	return bl1_tzram_layout;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	unsigned long bl1_normal_ram_base;
	unsigned long bl1_coherent_ram_limit;
	unsigned long tzram_limit = TZRAM_BASE + TZRAM_SIZE;

	/*
	 * Initialize extents of the bl1 sections as per the platform
	 * defined values.
	 */
	bl1_normal_ram_base  = BL1_NORMAL_RAM_BASE;
	bl1_coherent_ram_limit = BL1_NORMAL_RAM_LIMIT;

	/*
	 * Calculate how much ram is BL1 using & how much remains free.
	 * This also includes a rudimentary mechanism to detect whether
	 * the BL1 data is loaded at the top or bottom of memory.
	 * TODO: add support for discontigous chunks of free ram if
	 *       needed. Might need dynamic memory allocation support
	 *       et al.
	 *       Also assuming that the section for coherent memory is
	 *       the last and for globals the first in the scatter file.
	 */
	bl1_tzram_layout.total_base = TZRAM_BASE;
	bl1_tzram_layout.total_size = TZRAM_SIZE;

	if (bl1_coherent_ram_limit == tzram_limit) {
		bl1_tzram_layout.free_base = TZRAM_BASE;
		bl1_tzram_layout.free_size = bl1_normal_ram_base - TZRAM_BASE;
	} else {
		bl1_tzram_layout.free_base = bl1_coherent_ram_limit;
		bl1_tzram_layout.free_size =
			tzram_limit - bl1_coherent_ram_limit;
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
	/*
	 * This should zero out our coherent stacks as well but we don't care
	 * as they are not being used right now.
	 */
	memset((void *) BL1_FIRMWARE_RAM_COHERENT_ZI_BASE, 0,
	       (size_t) BL1_FIRMWARE_RAM_COHERENT_ZI_LENGTH);

	/* Enable and initialize the System level generic timer */
	mmio_write_32(SYS_CNTCTL_BASE + CNTCR_OFF, CNTCR_EN);

	/* Initialize the console */
	console_init();

	return;
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
		TZROM_BASE,			/* Read_only region start */
		TZROM_BASE + TZROM_SIZE,	/* Read_only region size */
		/* Coherent region start */
		BL1_FIRMWARE_RAM_COHERENT_ZI_BASE,
		/* Coherent region size */
		BL1_FIRMWARE_RAM_COHERENT_ZI_BASE +
			BL1_FIRMWARE_RAM_COHERENT_ZI_LENGTH);
}
