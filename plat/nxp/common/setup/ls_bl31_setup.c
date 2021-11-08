/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#ifdef LS_EL3_INTERRUPT_HANDLER
#include <ls_interrupt_mgmt.h>
#endif
#include <mmu_def.h>
#include <plat_common.h>

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
#ifdef TEST_BL31
#define  SPSR_FOR_EL2H   0x3C9
#define  SPSR_FOR_EL1H   0x3C5
#else
static entry_point_info_t bl31_image_ep_info;
#endif

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

static dram_regions_info_t dram_regions_info = {0};
static uint64_t rcw_porsr1;

/* Return the pointer to the 'dram_regions_info structure of the DRAM.
 * This structure is populated after init_ddr().
 */
dram_regions_info_t *get_dram_regions_info(void)
{
	return &dram_regions_info;
}

/* Return the RCW.PORSR1 value which was passed in from BL2
 */
uint64_t bl31_get_porsr1(void)
{
	return rcw_porsr1;
}

/*
 * Return pointer to the 'entry_point_info' structure of the next image for the
 * security state specified:
 * - BL33 corresponds to the non-secure image type; while
 * - BL32 corresponds to the secure image type.
 * - A NULL pointer is returned, if the image does not exist.
 */
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;

#ifdef TEST_BL31
	next_image_info->pc     = _get_test_entry();
	next_image_info->spsr   = SPSR_FOR_EL2H;
	next_image_info->h.attr = NON_SECURE;
#endif

	if (next_image_info->pc != 0U) {
		return next_image_info;
	} else {
		return NULL;
	}
}

/*
 * Perform any BL31 early platform setup common to NXP platforms.
 * - Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & S-EL3 in BL1) before they are lost (potentially).
 * - This needs to be done before the MMU is initialized so that the
 *   memory layout can be used while creating page tables.
 * - BL2 has flushed this information to memory, in order to fetch latest data.
 */

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
#ifndef TEST_BL31
	int i = 0;
	void *from_bl2 = (void *)arg0;
#endif
	soc_early_platform_setup2();

#ifdef TEST_BL31
	dram_regions_info.num_dram_regions  = 2;
	dram_regions_info.total_dram_size   = 0x100000000;
	dram_regions_info.region[0].addr    = 0x80000000;
	dram_regions_info.region[0].size    = 0x80000000;
	dram_regions_info.region[1].addr    = 0x880000000;
	dram_regions_info.region[1].size    = 0x80000000;

	bl33_image_ep_info.pc = _get_test_entry();
#else
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
		if (bl_params->image_id == BL31_IMAGE_ID) {
			bl31_image_ep_info = *bl_params->ep_info;
			dram_regions_info_t *loc_dram_regions_info =
			(dram_regions_info_t *) bl31_image_ep_info.args.arg3;

			dram_regions_info.num_dram_regions =
					loc_dram_regions_info->num_dram_regions;
			dram_regions_info.total_dram_size =
					loc_dram_regions_info->total_dram_size;
			VERBOSE("Number of DRAM Regions = %" PRIx64 "\n",
					dram_regions_info.num_dram_regions);

			for (i = 0; i < dram_regions_info.num_dram_regions;
									i++) {
				dram_regions_info.region[i].addr =
					loc_dram_regions_info->region[i].addr;
				dram_regions_info.region[i].size =
					loc_dram_regions_info->region[i].size;
				VERBOSE("DRAM%d Size = %" PRIx64 "\n", i,
					dram_regions_info.region[i].size);
			}
			rcw_porsr1 = bl31_image_ep_info.args.arg4;
		}

		if (bl_params->image_id == BL32_IMAGE_ID) {
			bl32_image_ep_info = *bl_params->ep_info;
		}

		if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
		}

		bl_params = bl_params->next_params_info;
	}
#endif /* TEST_BL31 */

	if (bl33_image_ep_info.pc == 0) {
		panic();
	}

	/*
	 * perform basic initialization on the soc
	 */
	soc_init();
}

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void bl31_platform_setup(void)
{
	NOTICE("Welcome to %s BL31 Phase\n", BOARD);
	soc_platform_setup();

	/* Console logs gone missing as part going to
	 * EL1 for initilizing Bl32 if present.
	 * console flush is necessary to avoid it.
	 */
	(void)console_flush();
}

void bl31_plat_runtime_setup(void)
{
#ifdef LS_EL3_INTERRUPT_HANDLER
	ls_el3_interrupt_config();
#endif
	soc_runtime_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl31_arch_setup()) does not do anything platform
 * specific.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
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
