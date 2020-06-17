/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <assert.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <plat_marvell.h>

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_get_ns_image_entrypoint
#pragma weak plat_marvell_get_mmap

/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The extents of the generic memory regions are specified by the function
 * arguments and consist of:
 * - Trusted SRAM seen by the BL image;
 * - Code section;
 * - Read-only data section;
 * - Coherent memory region, if applicable.
 */
void marvell_setup_page_tables(uintptr_t total_base,
			       size_t total_size,
			       uintptr_t code_start,
			       uintptr_t code_limit,
			       uintptr_t rodata_start,
			       uintptr_t rodata_limit
#if USE_COHERENT_MEM
			       ,
			       uintptr_t coh_start,
			       uintptr_t coh_limit
#endif
			   )
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Trusted SRAM seen by this BL image: %p - %p\n",
		(void *) total_base, (void *) (total_base + total_size));
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);

	/* Re-map the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *) code_start, (void *) code_limit);
	mmap_add_region(code_start, code_start,
			code_limit - code_start,
			MT_CODE | MT_SECURE);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
		(void *) rodata_start, (void *) rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
			rodata_limit - rodata_start,
			MT_RO_DATA | MT_SECURE);

#if USE_COHERENT_MEM
	/* Re-map the coherent memory region */
	VERBOSE("Coherent region: %p - %p\n",
		(void *) coh_start, (void *) coh_limit);
	mmap_add_region(coh_start, coh_start,
			coh_limit - coh_start,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	/* Now (re-)map the platform-specific memory regions */
	mmap_add(plat_marvell_get_mmap());

	/* Create the page tables to reflect the above mappings */
	init_xlat_tables();
}

unsigned long plat_get_ns_image_entrypoint(void)
{
	return PLAT_MARVELL_NS_IMAGE_OFFSET;
}

/*****************************************************************************
 * Gets SPSR for BL32 entry
 *****************************************************************************
 */
uint32_t marvell_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/*****************************************************************************
 * Gets SPSR for BL33 entry
 *****************************************************************************
 */
uint32_t marvell_get_spsr_for_bl33_entry(void)
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

/*****************************************************************************
 * Returns ARM platform specific memory map regions.
 *****************************************************************************
 */
const mmap_region_t *plat_marvell_get_mmap(void)
{
	return plat_marvell_mmap;
}

