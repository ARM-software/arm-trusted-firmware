/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ls_def.h>
#include <assert.h>
#include <console.h>
#include <mmio.h>
#include <plat_arm.h>
#include <plat_ls.h>
#include <soc.h>
#include <gicv2.h>
#include <uart_16550.h>
#define BL31_END (uintptr_t)(&__BL31_END__)

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

gicv2_driver_data_t arm_gic_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
};


/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;

	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & S-EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
void ls_bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2)
{
	static console_16550_t console;

	/* Initialize the console to provide early debug support */
	console_16550_register(LS_ATF_UART_BASE, LS_ATF_UART_CLOCK,
				LS_ATF_UART_BAUDRATE, &console);
#if RESET_TO_BL31
	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);

#ifdef BL32_BASE
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = arm_get_spsr_for_bl32_entry();
#endif /* BL32_BASE */

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();

	bl33_image_ep_info.spsr = arm_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#else /* RESET_TO_BL31 */

	/*
	 * In debug builds, we pass a special value in 'plat_params_from_bl2'
	 * to verify platform parameters from BL2 to BL31.
	 * In release builds, it's not used.
	 */
	assert(((unsigned long long)plat_params_from_bl2) ==
		ARM_BL31_PLAT_PARAM_VAL);

	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;

	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 and BL32 (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0)
		panic();

#endif /* RESET_TO_BL31 */
}

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void ls_bl31_platform_setup(void)
{
	uint32_t gicc_base, gicd_base;

	NOTICE(FIRMWARE_WELCOME_STR_LS1043_BL31);
	/* Initialize the GIC driver, cpu and distributor interfaces */
	get_gic_offset(&gicc_base, &gicd_base);
	arm_gic_data.gicd_base = (uintptr_t)gicd_base;
	arm_gic_data.gicc_base = (uintptr_t)gicc_base;
	gicv2_driver_init(&arm_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

#if RESET_TO_BL31
	/*
	 * Do initial security configuration to allow DRAM/device access
	 * (if earlier BL has not already done so).
	 */
	plat_arm_security_setup();

#endif /* RESET_TO_BL31 */

	/* Enable and initialize the System level generic timer */
	mmio_write_32(LS1043_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0) | CNTCR_EN);

	VERBOSE("Leave arm_bl31_platform_setup\n");
}

/*******************************************************************************
 * Perform any BL31 platform runtime setup prior to BL31 exit common to ARM
 * standard platforms
 ******************************************************************************/
void ls_bl31_plat_runtime_setup(void)
{
	static console_16550_t console;

	/* Initialize the runtime console */
	console_16550_register(PLAT_LS1043_UART_BASE, PLAT_LS1043_UART_CLOCK,
				PLAT_LS1043_UART_BAUDRATE, &console);
}

void bl31_platform_setup(void)
{
	ls_bl31_platform_setup();
}

void bl31_plat_runtime_setup(void)
{
	ls_bl31_plat_runtime_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl31_arch_setup()) does not do anything platform
 * specific.
 ******************************************************************************/
void ls_bl31_plat_arch_setup(void)
{
	ls_setup_page_tables(BL31_BASE,
			      BL31_END - BL31_BASE,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
#endif
			      );
	enable_mmu_el3(0);
}

void bl31_plat_arch_setup(void)
{
	ls_bl31_plat_arch_setup();
}
