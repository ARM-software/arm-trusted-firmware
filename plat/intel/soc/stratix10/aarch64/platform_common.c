/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <gicv2.h>
#include <bl_common.h>
#include <debug.h>
#include <xlat_tables.h>

#include <platform.h>
#include <platform_def.h>
#include <mmio.h>
//#include "Altera_Hps_Socal.h"

/* Table of regions to map using the MMU.  */
static const mmap_region_t plat_mmap[] = {
	MAP_REGION_FLAT(DRAM_BASE, DRAM_SIZE,
			MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE,
			MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE2_BASE, DEVICE2_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(OCRAM_BASE, OCRAM_SIZE,
			MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE3_BASE, DEVICE3_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MEM64_BASE, MEM64_SIZE,
			MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE4_BASE, DEVICE4_SIZE,
			MT_DEVICE | MT_RW | MT_NS),
	{ 0 }

};

/******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 *****************************************************************************/
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void plat_configure_mmu_el ## _el(unsigned long total_base,	\
					  unsigned long total_size,	\
					  unsigned long ro_start,	\
					  unsigned long ro_limit,	\
					  unsigned long coh_start,	\
					  unsigned long coh_limit)	\
	{		\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(ro_start, ro_start,			\
				ro_limit - ro_start,			\
				MT_MEMORY | MT_RO | MT_SECURE);		\
		mmap_add_region(coh_start, coh_start,			\
				coh_limit - coh_start,			\
				MT_DEVICE | MT_RW | MT_SECURE);		\
		mmap_add(plat_mmap);					\
		init_xlat_tables();					\
									\
		enable_mmu_el ## _el(0);				\
	}

/* Define EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(3)
DEFINE_CONFIGURE_MMU_EL(1)


unsigned int plat_get_syscnt_freq2(void)
{
	return PLAT_SYS_COUNTER_FREQ_IN_TICKS;
}

unsigned long plat_get_ns_image_entrypoint(void)
{
	return PLAT_NS_IMAGE_OFFSET;
}

/******************************************************************************
 * Gets SPSR for BL32 entry
 *****************************************************************************/
uint32_t plat_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/******************************************************************************
 * Gets SPSR for BL33 entry
 *****************************************************************************/
uint32_t plat_get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

