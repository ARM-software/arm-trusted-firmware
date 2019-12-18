/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include <bcm_console.h>
#include <plat_brcm.h>
#include <platform_def.h>

#ifdef BL33_SHARED_DDR_BASE
struct bl33_info *bl33_info = (struct bl33_info *)BL33_SHARED_DDR_BASE;
#endif

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/* Weak definitions may be overridden in specific BRCM platform */
#pragma weak plat_bcm_bl31_early_platform_setup
#pragma weak plat_brcm_pwrc_setup
#pragma weak plat_brcm_security_setup

void plat_brcm_security_setup(void)
{

}

void plat_brcm_pwrc_setup(void)
{

}

void plat_bcm_bl31_early_platform_setup(void *from_bl2,
				   bl_params_t *plat_params_from_bl2)
{

}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
struct entry_point_info *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;
	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
void __init brcm_bl31_early_platform_setup(void *from_bl2,
					  uintptr_t soc_fw_config,
					  uintptr_t hw_config,
					  void *plat_params_from_bl2)
{
	/* Initialize the console to provide early debug support */
	bcm_console_boot_init();

	/* Initialize delay timer driver using SP804 dual timer 0 */
	sp804_timer_init(SP804_TIMER0_BASE,
			 SP804_TIMER0_CLKMULT, SP804_TIMER0_CLKDIV);

#if RESET_TO_BL31
	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);

# ifdef BL32_BASE
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info,
		       PARAM_EP,
		       VERSION_1,
		       0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = brcm_get_spsr_for_bl32_entry();
# endif /* BL32_BASE */

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

	bl33_image_ep_info.spsr = brcm_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

# if ARM_LINUX_KERNEL_AS_BL33
	/*
	 * According to the file ``Documentation/arm64/booting.txt`` of the
	 * Linux kernel tree, Linux expects the physical address of the device
	 * tree blob (DTB) in x0, while x1-x3 are reserved for future use and
	 * must be 0.
	 */
	bl33_image_ep_info.args.arg0 = (u_register_t)PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
# endif

#else /* RESET_TO_BL31 */

	/*
	 * In debug builds, we pass a special value in 'plat_params_from_bl2'
	 * to verify platform parameters from BL2 to BL31.
	 * In release builds, it's not used.
	 */
	assert(((unsigned long long)plat_params_from_bl2) ==
		BRCM_BL31_PLAT_PARAM_VAL);

	/*
	 * Check params passed from BL2 should not be NULL
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
	while (bl_params != NULL) {
		if (bl_params->image_id == BL32_IMAGE_ID &&
		    bl_params->image_info->h.attr != IMAGE_ATTRIB_SKIP_LOADING)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0U)
		panic();
#endif /* RESET_TO_BL31 */

#ifdef BL33_SHARED_DDR_BASE
	/* Pass information to BL33 thorugh x0 */
	bl33_image_ep_info.args.arg0 = (u_register_t)BL33_SHARED_DDR_BASE;
	bl33_image_ep_info.args.arg1 = 0ULL;
	bl33_image_ep_info.args.arg2 = 0ULL;
	bl33_image_ep_info.args.arg3 = 0ULL;
#endif
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
#ifdef BL31_LOG_LEVEL
	SET_LOG_LEVEL(BL31_LOG_LEVEL);
#endif

	brcm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

	plat_bcm_bl31_early_platform_setup((void *)arg0, (void *)arg3);

#ifdef DRIVER_CC_ENABLE
	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_brcm_interconnect_init();

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * Platform specific PSCI code will enable coherency for other
	 * clusters.
	 */
	plat_brcm_interconnect_enter_coherency();
#endif
}

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void brcm_bl31_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_brcm_gic_driver_init();
	plat_brcm_gic_init();

	/* Initialize power controller before setting up topology */
	plat_brcm_pwrc_setup();
}

/*******************************************************************************
 * Perform any BL31 platform runtime setup prior to BL31 exit common to ARM
 * standard platforms
 * Perform BL31 platform setup
 ******************************************************************************/
void brcm_bl31_plat_runtime_setup(void)
{
	console_switch_state(CONSOLE_FLAG_RUNTIME);

	/* Initialize the runtime console */
	bcm_console_runtime_init();
}

void bl31_platform_setup(void)
{
	brcm_bl31_platform_setup();

	/* Initialize the secure environment */
	plat_brcm_security_setup();
}

void bl31_plat_runtime_setup(void)
{
	brcm_bl31_plat_runtime_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl31_arch_setup()) does not do anything platform
 * specific.
 ******************************************************************************/
void __init brcm_bl31_plat_arch_setup(void)
{
#ifndef MMU_DISABLED
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL31_BASE, BL31_END - BL31_BASE,
				MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
				MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE,
				BL_RO_DATA_END - BL_RO_DATA_BASE,
				MT_RO_DATA | MT_SECURE),
#if USE_COHERENT_MEM
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
				BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
				MT_DEVICE | MT_RW | MT_SECURE),
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_brcm_get_mmap());

	enable_mmu_el3(0);
#endif
}

void __init bl31_plat_arch_setup(void)
{
	brcm_bl31_plat_arch_setup();
}
