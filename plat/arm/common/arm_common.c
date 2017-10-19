/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
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
#include <secure_partition.h>

extern const mmap_region_t plat_arm_mmap[];

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_get_ns_image_entrypoint
#pragma weak plat_arm_get_mmap

/* Conditionally provide a weak definition of plat_get_syscnt_freq2 to avoid
 * conflicts with the definition in plat/common. */
#if ERROR_DEPRECATED
#pragma weak plat_get_syscnt_freq2
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
void arm_setup_page_tables(uintptr_t total_base,
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

#if ENABLE_SPM && defined(IMAGE_BL31)
	/* The address of the following region is calculated by the linker. */
	mmap_add_region(SP_IMAGE_XLAT_TABLES_START,
			SP_IMAGE_XLAT_TABLES_START,
			SP_IMAGE_XLAT_TABLES_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE);
#endif

	/* Now (re-)map the platform-specific memory regions */
	mmap_add(plat_arm_get_mmap());

	/* Create the page tables to reflect the above mappings */
	init_xlat_tables();
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_ARM_NS_IMAGE_OFFSET;
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
	mmio_write_32(ARM_SYS_TIMCTL_BASE + CNTACR_BASE(PLAT_ARM_NSTIMER_FRAME_ID), reg_val);
#endif /* ARM_CONFIG_CNTACR */

	reg_val = (1 << CNTNSAR_NS_SHIFT(PLAT_ARM_NSTIMER_FRAME_ID));
	mmio_write_32(ARM_SYS_TIMCTL_BASE + CNTNSAR, reg_val);
}
#endif /* ARM_SYS_TIMCTL_BASE */

/*******************************************************************************
 * Returns ARM platform specific memory map regions.
 ******************************************************************************/
const mmap_region_t *plat_arm_get_mmap(void)
{
	return plat_arm_mmap;
}

#ifdef ARM_SYS_CNTCTL_BASE

unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;

	/* Read the frequency from Frequency modes table */
	counter_base_frequency = mmio_read_32(ARM_SYS_CNTCTL_BASE + CNTFID_OFF);

	/* The first entry of the frequency modes table must not be 0 */
	if (counter_base_frequency == 0)
		panic();

	return counter_base_frequency;
}

#endif /* ARM_SYS_CNTCTL_BASE */

#if SDEI_SUPPORT
/*
 * Translate SDEI entry point to PA, and perform standard ARM entry point
 * validation on it.
 */
int plat_sdei_validate_entry_point(uintptr_t ep, unsigned int client_mode)
{
	uint64_t par, pa;
	uint32_t scr_el3;

	/* Doing Non-secure address translation requires SCR_EL3.NS set */
	scr_el3 = read_scr_el3();
	write_scr_el3(scr_el3 | SCR_NS_BIT);
	isb();

	assert((client_mode == MODE_EL2) || (client_mode == MODE_EL1));
	if (client_mode == MODE_EL2) {
		/*
		 * Translate entry point to Physical Address using the EL2
		 * translation regime.
		 */
		ats1e2r(ep);
	} else {
		/*
		 * Translate entry point to Physical Address using the EL1&0
		 * translation regime, including stage 2.
		 */
		ats12e1r(ep);
	}
	isb();
	par = read_par_el1();

	/* Restore original SCRL_EL3 */
	write_scr_el3(scr_el3);
	isb();

	/* If the translation resulted in fault, return failure */
	if ((par & PAR_F_MASK) != 0)
		return -1;

	/* Extract Physical Address from PAR */
	pa = (par & (PAR_ADDR_MASK << PAR_ADDR_SHIFT));

	/* Perform NS entry point validation on the physical address */
	return arm_validate_ns_entrypoint(pa);
}
#endif
