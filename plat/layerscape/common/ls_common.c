/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <arch_helpers.h>
#include <arm_xlat_tables.h>
#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <plat_arm.h>
#include <platform_def.h>

const mmap_region_t *plat_ls_get_mmap(void);

/*
 * Table of memory regions for various BL stages to map using the MMU.
 * This doesn't include Trusted SRAM as arm_setup_page_tables() already
 * takes care of mapping it.
 *
 * The flash needs to be mapped as writable in order to erase the FIP's Table of
 * Contents in case of unrecoverable error (see plat_error_handler()).
 */
#ifdef IMAGE_BL1
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_FLASH0_RW,
//	LS_MAP_CONSOLE,
	LS_MAP_NS_DRAM,
	LS_MAP_CCSR,
	{0}
};
#endif
#ifdef IMAGE_BL2
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_FLASH0_RW,
	LS_MAP_CCSR,
	LS_MAP_NS_DRAM,
	LS_MAP_TSP_SEC_MEM,
	{0}
};
#endif
#ifdef IMAGE_BL31
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_CCSR,
//	LS_MAP_CONSOLE,
	LS_MAP_FLASH0_RW,
	LS_MAP_NS_DRAM,
	LS_MAP_TSP_SEC_MEM,
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_CCSR,
	LS_MAP_FLASH0_RW,
//	LS_MAP_NS_DRAM,
	LS_MAP_TSP_SEC_MEM,
	{0}
};
#endif
/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The extents of the generic memory regions are specified by the function
 * arguments and consist of:
 * - Trusted SRAM seen by the BL image;
 * - Code section;
 * - Read-only data section;
 * - Coherent memory region, if applicable.
 */
void ls_setup_page_tables(uintptr_t total_base,
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
	mmap_add(plat_ls_get_mmap());
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
//	mmap_add(plat_ls_get_mmap());

	/* Create the page tables to reflect the above mappings */
	init_xlat_tables();
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return LS_NS_DRAM_BASE;
#endif
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t arm_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
#ifndef AARCH32
uint32_t arm_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = EL_IMPLEMENTED(2) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#else
/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
uint32_t arm_get_spsr_for_bl33_entry(void)
{
	unsigned int hyp_status, mode, spsr;

	hyp_status = GET_VIRT_EXT(read_id_pfr1());

	mode = (hyp_status) ? MODE32_hyp : MODE32_svc;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_MODE32(mode, plat_get_ns_image_entrypoint() & 0x1,
			SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#endif /* AARCH32 */

/*******************************************************************************
 * Configures access to the system counter timer module.
 ******************************************************************************/
#ifdef ARM_SYS_TIMCTL_BASE
void arm_configure_sys_timer(void)
{
	unsigned int reg_val;

#if ARM_CONFIG_CNTACR
	reg_val = (1 << CNTACR_RPCT_SHIFT) | (1 << CNTACR_RVCT_SHIFT);
	reg_val |= (1 << CNTACR_RFRQ_SHIFT) | (1 << CNTACR_RVOFF_SHIFT);
	reg_val |= (1 << CNTACR_RWVT_SHIFT) | (1 << CNTACR_RWPT_SHIFT);
	mmio_write_32(ARM_SYS_TIMCTL_BASE +
			CNTACR_BASE(PLAT_ARM_NSTIMER_FRAME_ID), reg_val);
#endif /* ARM_CONFIG_CNTACR */

	reg_val = (1 << CNTNSAR_NS_SHIFT(PLAT_ARM_NSTIMER_FRAME_ID));
	mmio_write_32(ARM_SYS_TIMCTL_BASE + CNTNSAR, reg_val);
}
#endif /* ARM_SYS_TIMCTL_BASE */

/*******************************************************************************
 * Returns ARM platform specific memory map regions.
 ******************************************************************************/
const mmap_region_t *plat_ls_get_mmap(void)
{
	return plat_ls_mmap;
}


unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;

	counter_base_frequency = COUNTER_FREQUENCY;

	return counter_base_frequency;
}
