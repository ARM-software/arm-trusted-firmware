/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/romlib.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <services/arm_arch_svc.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_get_ns_image_entrypoint
#pragma weak plat_arm_get_mmap

/* Conditionally provide a weak definition of plat_get_syscnt_freq2 to avoid
 * conflicts with the definition in plat/common. */
#pragma weak plat_get_syscnt_freq2

/* Get ARM SOC-ID */
#pragma weak plat_arm_get_soc_id

/*******************************************************************************
 * Changes the memory attributes for the region of mapped memory where the BL
 * image's translation tables are located such that the tables will have
 * read-only permissions.
 ******************************************************************************/
#if PLAT_RO_XLAT_TABLES
void arm_xlat_make_tables_readonly(void)
{
	int rc = xlat_make_tables_readonly();

	if (rc != 0) {
		ERROR("Failed to make translation tables read-only at EL%u.\n",
		      get_current_el());
		panic();
	}

	INFO("Translation tables are now read-only at EL%u.\n",
	     get_current_el());
}
#endif

void arm_setup_romlib(void)
{
#if USE_ROMLIB
	if (!rom_lib_init(ROMLIB_VERSION))
		panic();
#endif
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_ARM_NS_IMAGE_BASE;
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
#ifdef __aarch64__
uint32_t arm_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64((uint64_t)mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
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
#endif /* __aarch64__ */

/*******************************************************************************
 * Configures access to the system counter timer module.
 ******************************************************************************/
#ifdef ARM_SYS_TIMCTL_BASE
void arm_configure_sys_timer(void)
{
	unsigned int reg_val;

	/* Read the frequency of the system counter */
	unsigned int freq_val = plat_get_syscnt_freq2();

#if ARM_CONFIG_CNTACR
	reg_val = (1U << CNTACR_RPCT_SHIFT) | (1U << CNTACR_RVCT_SHIFT);
	reg_val |= (1U << CNTACR_RFRQ_SHIFT) | (1U << CNTACR_RVOFF_SHIFT);
	reg_val |= (1U << CNTACR_RWVT_SHIFT) | (1U << CNTACR_RWPT_SHIFT);
	mmio_write_32(ARM_SYS_TIMCTL_BASE + CNTACR_BASE(PLAT_ARM_NSTIMER_FRAME_ID), reg_val);
#endif /* ARM_CONFIG_CNTACR */

	reg_val = (1U << CNTNSAR_NS_SHIFT(PLAT_ARM_NSTIMER_FRAME_ID));
	mmio_write_32(ARM_SYS_TIMCTL_BASE + CNTNSAR, reg_val);

	/*
	 * Initialize CNTFRQ register in CNTCTLBase frame. The CNTFRQ
	 * system register initialized during psci_arch_setup() is different
	 * from this and has to be updated independently.
	 */
	mmio_write_32(ARM_SYS_TIMCTL_BASE + CNTCTLBASE_CNTFRQ, freq_val);

#if defined(PLAT_juno) || defined(PLAT_n1sdp)
	/*
	 * Initialize CNTFRQ register in Non-secure CNTBase frame.
	 * This is only required for Juno and N1SDP, because they do not
	 * follow ARM ARM in that the value updated in CNTFRQ is not
	 * reflected in CNTBASEN_CNTFRQ. Hence update the value manually.
	 */
	mmio_write_32(ARM_SYS_CNT_BASE_NS + CNTBASEN_CNTFRQ, freq_val);
#endif
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
	if (counter_base_frequency == 0U)
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
	u_register_t scr_el3;

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
		AT(ats12e1r, ep);
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

