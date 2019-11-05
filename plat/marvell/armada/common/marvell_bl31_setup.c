/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <assert.h>

#include <arch.h>
#include <common/bl_common.h>
#include <common/debug.h>
#ifdef USE_CCI
#include <drivers/arm/cci.h>
#endif
#include <drivers/console.h>
#include <plat/common/platform.h>

#include <marvell_def.h>
#include <marvell_plat_priv.h>
#include <plat_marvell.h>

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak bl31_early_platform_setup2
#pragma weak bl31_platform_setup
#pragma weak bl31_plat_arch_setup
#pragma weak bl31_plat_get_next_image_ep_info
#pragma weak plat_get_syscnt_freq2

/*****************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 *****************************************************************************
 */
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;

	return next_image_info;
}

/*****************************************************************************
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 *****************************************************************************
 */
void marvell_bl31_early_platform_setup(void *from_bl2,
				       uintptr_t soc_fw_config,
				       uintptr_t hw_config,
				       void *plat_params_from_bl2)
{
	/* Initialize the console to provide early debug support */
	marvell_console_boot_init();

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
	bl32_image_ep_info.spsr = marvell_get_spsr_for_bl32_entry();
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
	bl33_image_ep_info.spsr = marvell_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#else
	/*
	 * In debug builds, we pass a special value in 'plat_params_from_bl2'
	 * to verify platform parameters from BL2 to BL31.
	 * In release builds, it's not used.
	 */
	assert(((unsigned long long)plat_params_from_bl2) ==
		MARVELL_BL31_PLAT_PARAM_VAL);

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
	while (bl_params != NULL) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}
#endif
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)

{
	marvell_bl31_early_platform_setup((void *)arg0, arg1, arg2,
					  (void *)arg3);

#ifdef USE_CCI
	/*
	 * Initialize CCI for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_marvell_interconnect_init();

	/*
	 * Enable CCI coherency for the primary CPU's cluster.
	 * Platform specific PSCI code will enable coherency for other
	 * clusters.
	 */
	plat_marvell_interconnect_enter_coherency();
#endif
}

/*****************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 *****************************************************************************
 */
void marvell_bl31_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_marvell_gic_driver_init();
	plat_marvell_gic_init();

	/* For Armada-8k-plus family, the SoC includes more than
	 * a single AP die, but the default die that boots is AP #0.
	 * For other families there is only one die (#0).
	 * Initialize psci arch from die 0
	 */
	marvell_psci_arch_init(0);
}

/*****************************************************************************
 * Perform any BL31 platform runtime setup prior to BL31 exit common to ARM
 * standard platforms
 *****************************************************************************
 */
void marvell_bl31_plat_runtime_setup(void)
{
	console_switch_state(CONSOLE_FLAG_RUNTIME);

	/* Initialize the runtime console */
	marvell_console_runtime_init();
}

void bl31_platform_setup(void)
{
	marvell_bl31_platform_setup();
}

void bl31_plat_runtime_setup(void)
{
	marvell_bl31_plat_runtime_setup();
}

/*****************************************************************************
 * Perform the very early platform specific architectural setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl31_arch_setup()) does not do anything platform
 * specific.
 *****************************************************************************
 */
void marvell_bl31_plat_arch_setup(void)
{
	marvell_setup_page_tables(BL31_BASE,
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

#if BL31_CACHE_DISABLE
	enable_mmu_el3(DISABLE_DCACHE);
	INFO("Cache is disabled in BL3\n");
#else
	enable_mmu_el3(0);
#endif
}

void bl31_plat_arch_setup(void)
{
	marvell_bl31_plat_arch_setup();
}

unsigned int plat_get_syscnt_freq2(void)
{
	return PLAT_REF_CLK_IN_HZ;
}
