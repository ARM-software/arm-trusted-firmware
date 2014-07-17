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

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <xlat_tables.h>
#include "../juno_def.h"

/*
 * Table of regions to map using the MMU.
 * This doesn't include Trusted RAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
static const mmap_region_t juno_mmap[] = {
	{ TZROM_BASE,		TZROM_BASE,		TZROM_SIZE,		MT_MEMORY | MT_RO | MT_SECURE },
	{ MHU_SECURE_BASE,	MHU_SECURE_BASE,	MHU_SECURE_SIZE,	(MHU_PAYLOAD_CACHED ? MT_MEMORY : MT_DEVICE) | MT_RW | MT_SECURE },
	{ FLASH_BASE,		FLASH_BASE,		FLASH_SIZE,		MT_MEMORY | MT_RO | MT_SECURE },
	{ EMMC_BASE,		EMMC_BASE,		EMMC_SIZE,		MT_MEMORY | MT_RO | MT_SECURE },
	{ PSRAM_BASE,		PSRAM_BASE,		PSRAM_SIZE,		MT_MEMORY | MT_RW | MT_SECURE }, /* Used for 'TZDRAM' */
	{ IOFPGA_BASE,		IOFPGA_BASE,		IOFPGA_SIZE,		MT_DEVICE | MT_RW | MT_SECURE },
	{ DEVICE0_BASE,		DEVICE0_BASE,		DEVICE0_SIZE,		MT_DEVICE | MT_RW | MT_SECURE },
	{ DEVICE1_BASE,		DEVICE1_BASE,		DEVICE1_SIZE,		MT_DEVICE | MT_RW | MT_SECURE },
	{ DRAM_BASE,		DRAM_BASE,		DRAM_SIZE,		MT_MEMORY | MT_RW | MT_NS },
	{0}
};

/*******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 ******************************************************************************/
#define DEFINE_CONFIGURE_MMU_EL(_el)				\
	void configure_mmu_el##_el(unsigned long total_base,	\
				  unsigned long total_size,	\
				  unsigned long ro_start,	\
				  unsigned long ro_limit,	\
				  unsigned long coh_start,	\
				  unsigned long coh_limit)	\
	{							\
	       mmap_add_region(total_base, total_base,		\
			       total_size,			\
			       MT_MEMORY | MT_RW | MT_SECURE);	\
	       mmap_add_region(ro_start, ro_start,		\
			       ro_limit - ro_start,		\
			       MT_MEMORY | MT_RO | MT_SECURE);	\
	       mmap_add_region(coh_start, coh_start,		\
			       coh_limit - coh_start,		\
			       MT_DEVICE | MT_RW | MT_SECURE);	\
	       mmap_add(juno_mmap);				\
	       init_xlat_tables();				\
								\
	       enable_mmu_el##_el(0);				\
	}

/* Define EL1 and EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(1)
DEFINE_CONFIGURE_MMU_EL(3)


unsigned long plat_get_ns_image_entrypoint(void)
{
	return NS_IMAGE_OFFSET;
}

uint64_t plat_get_syscnt_freq(void)
{
	uint64_t counter_base_frequency;

	/* Read the frequency from Frequency modes table */
	counter_base_frequency = mmio_read_32(SYS_CNTCTL_BASE + CNTFID_OFF);

	/* The first entry of the frequency modes table must not be 0 */
	if (counter_base_frequency == 0)
		panic();

	return counter_base_frequency;
}
