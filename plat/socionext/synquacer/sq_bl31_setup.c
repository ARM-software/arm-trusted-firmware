/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <lib/mmio.h>
#include <sq_common.h>

static console_pl011_t console;
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

IMPORT_SYM(uintptr_t, __SPM_SHIM_EXCEPTIONS_START__, SPM_SHIM_EXCEPTIONS_START);
IMPORT_SYM(uintptr_t, __SPM_SHIM_EXCEPTIONS_END__,   SPM_SHIM_EXCEPTIONS_END);
IMPORT_SYM(uintptr_t, __SPM_SHIM_EXCEPTIONS_LMA__,   SPM_SHIM_EXCEPTIONS_LMA);

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type));
	return type == NON_SECURE ? &bl33_image_ep_info : &bl32_image_ep_info;
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t sq_get_spsr_for_bl32_entry(void)
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
uint32_t sq_get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	(void)console_pl011_register(PLAT_SQ_BOOT_UART_BASE,
			       PLAT_SQ_BOOT_UART_CLK_IN_HZ,
			       SQ_CONSOLE_BAUDRATE, &console);

	console_set_scope(&console.console, CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME);

	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(arg0 == 0U);
	assert(arg1 == 0U);

	/* Initialize power controller before setting up topology */
	plat_sq_pwrc_setup();

#ifdef SPD_opteed
	struct draminfo di = {0};

	sq_scp_get_draminfo(&di);

	/*
	 * Check if OP-TEE has been loaded in Secure RAM allocated
	 * from DRAM1 region
	 */
	if ((di.base1 + di.size1) <= BL32_BASE) {
		NOTICE("OP-TEE has been loaded by SCP firmware\n");
		/* Populate entry point information for BL32 */
		SET_PARAM_HEAD(&bl32_image_ep_info,
					PARAM_EP,
					VERSION_1,
					0);
		SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
		bl32_image_ep_info.pc = BL32_BASE;
		bl32_image_ep_info.spsr = sq_get_spsr_for_bl32_entry();
	} else {
		NOTICE("OP-TEE has not been loaded by SCP firmware\n");
	}
#endif /* SPD_opteed */

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = PRELOADED_BL33_BASE;
	bl33_image_ep_info.spsr = sq_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
}

static void sq_configure_sys_timer(void)
{
	unsigned int reg_val;

	reg_val = (1 << CNTACR_RPCT_SHIFT) | (1 << CNTACR_RVCT_SHIFT);
	reg_val |= (1 << CNTACR_RFRQ_SHIFT) | (1 << CNTACR_RVOFF_SHIFT);
	reg_val |= (1 << CNTACR_RWVT_SHIFT) | (1 << CNTACR_RWPT_SHIFT);
	mmio_write_32(SQ_SYS_TIMCTL_BASE +
		      CNTACR_BASE(PLAT_SQ_NSTIMER_FRAME_ID), reg_val);

	reg_val = (1 << CNTNSAR_NS_SHIFT(PLAT_SQ_NSTIMER_FRAME_ID));
	mmio_write_32(SQ_SYS_TIMCTL_BASE + CNTNSAR, reg_val);
}

void bl31_platform_setup(void)
{
	/* Initialize the CCN interconnect */
	plat_sq_interconnect_init();
	plat_sq_interconnect_enter_coherency();

	/* Initialize the GIC driver, cpu and distributor interfaces */
	sq_gic_driver_init();
	sq_gic_init();

	/* Enable and initialize the System level generic timer */
	mmio_write_32(SQ_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0U) | CNTCR_EN);

	/* Allow access to the System counter timer module */
	sq_configure_sys_timer();
}

void bl31_plat_runtime_setup(void)
{
	struct draminfo *di = (struct draminfo *)(unsigned long)DRAMINFO_BASE;

	sq_scp_get_draminfo(di);
}

void bl31_plat_arch_setup(void)
{
	static const mmap_region_t secure_partition_mmap[] = {
#if SPM_MM
		MAP_REGION_FLAT(PLAT_SPM_BUF_BASE,
				PLAT_SPM_BUF_SIZE,
				MT_RW_DATA | MT_SECURE),
		MAP_REGION_FLAT(PLAT_SQ_SP_PRIV_BASE,
				PLAT_SQ_SP_PRIV_SIZE,
				MT_RW_DATA | MT_SECURE),
#endif
		{0},
	};

	sq_mmap_setup(BL31_BASE, BL31_SIZE, secure_partition_mmap);
	enable_mmu_el3(XLAT_TABLE_NC);

#if SPM_MM
	memcpy((void *)SPM_SHIM_EXCEPTIONS_START,
	       (void *)SPM_SHIM_EXCEPTIONS_LMA,
	       (uintptr_t)SPM_SHIM_EXCEPTIONS_END -
	       (uintptr_t)SPM_SHIM_EXCEPTIONS_START);
#endif
}

void bl31_plat_enable_mmu(uint32_t flags)
{
	enable_mmu_el3(flags | XLAT_TABLE_NC);
}

unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;

	/* Read the frequency from Frequency modes table */
	counter_base_frequency = mmio_read_32(SQ_SYS_CNTCTL_BASE + CNTFID_OFF);

	/* The first entry of the frequency modes table must not be 0 */
	if (counter_base_frequency == 0)
		panic();

	return counter_base_frequency;
}
