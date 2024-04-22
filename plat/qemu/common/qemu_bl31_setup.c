/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <drivers/arm/pl061_gpio.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/transfer_list.h>
#include <plat/common/platform.h>

#include "qemu_private.h"

#define MAP_BL31_TOTAL		MAP_REGION_FLAT(			\
					BL31_BASE,			\
					BL31_END - BL31_BASE,		\
					MT_MEMORY | MT_RW | EL3_PAS)
#define MAP_BL31_RO		MAP_REGION_FLAT(			\
					BL_CODE_BASE,			\
					BL_CODE_END - BL_CODE_BASE,	\
					MT_CODE | EL3_PAS),		\
				MAP_REGION_FLAT(			\
					BL_RO_DATA_BASE,		\
					BL_RO_DATA_END			\
						- BL_RO_DATA_BASE,	\
					MT_RO_DATA | EL3_PAS)

#if USE_COHERENT_MEM
#define MAP_BL_COHERENT_RAM	MAP_REGION_FLAT(			\
					BL_COHERENT_RAM_BASE,		\
					BL_COHERENT_RAM_END		\
						- BL_COHERENT_RAM_BASE,	\
					MT_DEVICE | MT_RW | EL3_PAS)
#endif

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL3-1 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
#if ENABLE_RME
static entry_point_info_t rmm_image_ep_info;
#endif
static struct transfer_list_header *bl31_tl;

/*******************************************************************************
 * Perform any BL3-1 early platform setup.  Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & EL3 in BL1) before
 * they are lost (potentially). This needs to be done before the MMU is
 * initialized so that the memory layout can be used while creating page
 * tables. BL2 has flushed this information to memory, so we are guaranteed
 * to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	qemu_console_init();

/* Platform names have to be lowercase. */
#ifdef PLAT_qemu_sbsa
	sip_svc_init();
#endif

	/*
	 * Check params passed from BL2
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)arg0;

	assert(params_from_bl2);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33, BL32 and RMM (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

#if ENABLE_RME
		if (bl_params->image_id == RMM_IMAGE_ID)
			rmm_image_ep_info = *bl_params->ep_info;
#endif

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (!bl33_image_ep_info.pc)
		panic();
#if ENABLE_RME
	if (!rmm_image_ep_info.pc)
		panic();
#endif

	if (TRANSFER_LIST && arg1 == (TRANSFER_LIST_SIGNATURE |
				      REGISTER_CONVENTION_VERSION_MASK) &&
	    transfer_list_check_header((void *)arg3) != TL_OPS_NON) {
		bl31_tl = (void *)arg3; /* saved TL address from BL2 */
	}
}

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL31_TOTAL,
		MAP_BL31_RO,
#if USE_COHERENT_MEM
		MAP_BL_COHERENT_RAM,
#endif
#if ENABLE_RME
		MAP_GPT_L0_REGION,
		MAP_GPT_L1_REGION,
		MAP_RMM_SHARED_MEM,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_qemu_get_mmap());

	enable_mmu_el3(0);

#if ENABLE_RME
	/*
	 * Initialise Granule Protection library and enable GPC for the primary
	 * processor. The tables have already been initialized by a previous BL
	 * stage, so there is no need to provide any PAS here. This function
	 * sets up pointers to those tables.
	 */
	if (gpt_runtime_init() < 0) {
		ERROR("gpt_runtime_init() failed!\n");
		panic();
	}
#endif /* ENABLE_RME */

}

static void qemu_gpio_init(void)
{
#ifdef SECURE_GPIO_BASE
	pl061_gpio_init();
	pl061_gpio_register(SECURE_GPIO_BASE, 0);
#endif
}

void bl31_platform_setup(void)
{
	plat_qemu_gic_init();
	qemu_gpio_init();
}

unsigned int plat_get_syscnt_freq2(void)
{
	return read_cntfrq_el0();
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image
 * for the security state specified. BL3-3 corresponds to the non-secure
 * image type while BL3-2 corresponds to the secure image type. A NULL
 * pointer is returned if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	if (type == NON_SECURE) {
		next_image_info = &bl33_image_ep_info;
	}
#if ENABLE_RME
	else if (type == REALM) {
		next_image_info = &rmm_image_ep_info;
	}
#endif
	else {
		next_image_info =  &bl32_image_ep_info;
	}

	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

void bl31_plat_runtime_setup(void)
{
#if TRANSFER_LIST
	if (bl31_tl) {
		/*
		 * update the TL from S to NS memory before jump to BL33
		 * to reflect all changes in TL done by BL32
		 */
		memcpy((void *)FW_NS_HANDOFF_BASE, bl31_tl, bl31_tl->max_size);
	}
#endif

	console_flush();
	console_switch_state(CONSOLE_FLAG_RUNTIME);
}
