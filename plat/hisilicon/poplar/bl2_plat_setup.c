/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <desc_image_load.h>
#include <dw_mmc.h>
#include <errno.h>
#include <generic_delay_timer.h>
#include <mmc.h>
#include <mmio.h>
#include <optee_utils.h>
#include <partition/partition.h>
#include <platform.h>
#include <string.h>
#include "hi3798cv200.h"
#include "plat_private.h"

/* Memory ranges for code and read only data sections */
#define BL2_RO_BASE	(unsigned long)(&__RO_START__)
#define BL2_RO_LIMIT	(unsigned long)(&__RO_END__)

/* Memory ranges for coherent memory section */
#define BL2_COHERENT_RAM_BASE	(unsigned long)(&__COHERENT_RAM_START__)
#define BL2_COHERENT_RAM_LIMIT	(unsigned long)(&__COHERENT_RAM_END__)

static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

#if !LOAD_IMAGE_V2

/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL31, e.g. while passing control to it from BL2, bl31_params
 * and other platform specific params
 ******************************************************************************/
typedef struct bl2_to_bl31_params_mem {
	bl31_params_t		bl31_params;
	image_info_t		bl31_image_info;
	image_info_t		bl32_image_info;
	image_info_t		bl33_image_info;
	entry_point_info_t	bl33_ep_info;
	entry_point_info_t	bl32_ep_info;
	entry_point_info_t	bl31_ep_info;
} bl2_to_bl31_params_mem_t;

static bl2_to_bl31_params_mem_t bl31_params_mem;

meminfo_t *bl2_plat_sec_mem_layout(void)
{
	return &bl2_tzram_layout;
}

#ifdef SCP_BL2_BASE
void bl2_plat_get_scp_bl2_meminfo(meminfo_t *scp_bl2_meminfo)
{
	/*
	 * This platform has no SCP_BL2 yet
	 */
}
#endif
#endif /* LOAD_IMAGE_V2 */

/*******************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 ******************************************************************************/
#if LOAD_IMAGE_V2
int plat_poplar_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
#else
int bl2_plat_handle_scp_bl2(struct image_info *scp_bl2_image_info)
#endif
{
	/*
	 * This platform has no SCP_BL2 yet
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t poplar_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL3-2 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
#ifndef AARCH32
uint32_t poplar_get_spsr_for_bl33_entry(void)
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
#else
uint32_t poplar_get_spsr_for_bl33_entry(void)
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

#if LOAD_IMAGE_V2
int poplar_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
#ifdef SPD_opteed
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif

	assert(bl_mem_params);

	switch (image_id) {
#ifdef AARCH64
	case BL32_IMAGE_ID:
#ifdef SPD_opteed
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
				&pager_mem_params->image_info,
				&paged_mem_params->image_info);
		if (err != 0) {
			WARN("OPTEE header parse error.\n");
		}

		/*
		 * OP-TEE expect to receive DTB address in x2.
		 * This will be copied into x2 by dispatcher.
		 * Set this (arg3) if necessary
		 */
		/* bl_mem_params->ep_info.args.arg3 = PLAT_HIKEY_DT_BASE; */
#endif
		bl_mem_params->ep_info.spsr = poplar_get_spsr_for_bl32_entry();
		break;
#endif

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = poplar_get_spsr_for_bl33_entry();
		break;

#ifdef SCP_BL2_BASE
	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific */
		err = plat_poplar_bl2_handle_scp_bl2(&bl_mem_params->image_info);
		if (err) {
			WARN("Failure in platform-specific handling of SCP_BL2 image.\n");
		}
		break;
#endif
	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return poplar_bl2_handle_post_image_load(image_id);
}

#else /* LOAD_IMAGE_V2 */

bl31_params_t *bl2_plat_get_bl31_params(void)
{
	bl31_params_t *bl2_to_bl31_params = NULL;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL3-1
	 */
	memset(&bl31_params_mem, 0, sizeof(bl2_to_bl31_params_mem_t));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, PARAM_BL31, VERSION_1, 0);

	/* Fill BL3-1 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       PARAM_IMAGE_BINARY, VERSION_1, 0);

	/* Fill BL3-2 related information if it exists */
#ifdef BL32_BASE
	bl2_to_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_ep_info, PARAM_EP,
		VERSION_1, 0);
	bl2_to_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);
#endif

	/* Fill BL3-3 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_ep_info,
		       PARAM_EP, VERSION_1, 0);

	/* BL3-3 expects to receive the primary CPU MPID (through x0) */
	bl2_to_bl31_params->bl33_ep_info->args.arg0 = 0xffff & read_mpidr();

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info,
		       PARAM_IMAGE_BINARY, VERSION_1, 0);

	return bl2_to_bl31_params;
}

struct entry_point_info *bl2_plat_get_bl31_ep_info(void)
{
#if DEBUG
	bl31_params_mem.bl31_ep_info.args.arg1 = POPLAR_BL31_PLAT_PARAM_VAL;
#endif

	return &bl31_params_mem.bl31_ep_info;
}

void bl2_plat_set_bl31_ep_info(image_info_t *image,
			       entry_point_info_t *bl31_ep_info)
{
	SET_SECURITY_STATE(bl31_ep_info->h.attr, SECURE);
	bl31_ep_info->spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
				     DISABLE_ALL_EXCEPTIONS);
}

/*******************************************************************************
 * Before calling this function BL32 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL32 and set SPSR and security state.
 * On Poplar we only set the security state of the entrypoint
 ******************************************************************************/
#ifdef BL32_BASE
void bl2_plat_set_bl32_ep_info(image_info_t *bl32_image_info,
					entry_point_info_t *bl32_ep_info)
{
	SET_SECURITY_STATE(bl32_ep_info->h.attr, SECURE);
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	bl32_ep_info->spsr = 0;
}

/*******************************************************************************
 * Populate the extents of memory available for loading BL32
 ******************************************************************************/
void bl2_plat_get_bl32_meminfo(meminfo_t *bl32_meminfo)
{
	/*
	 * Populate the extents of memory available for loading BL32.
	 */
	bl32_meminfo->total_base = BL32_BASE;
	bl32_meminfo->free_base = BL32_BASE;
	bl32_meminfo->total_size =
			(TSP_SEC_MEM_BASE + TSP_SEC_MEM_SIZE) - BL32_BASE;
	bl32_meminfo->free_size =
			(TSP_SEC_MEM_BASE + TSP_SEC_MEM_SIZE) - BL32_BASE;
}
#endif /* BL32_BASE */

void bl2_plat_set_bl33_ep_info(image_info_t *image,
			       entry_point_info_t *bl33_ep_info)
{
	SET_SECURITY_STATE(bl33_ep_info->h.attr, NON_SECURE);
	bl33_ep_info->spsr = poplar_get_spsr_for_bl33_entry();
	bl33_ep_info->args.arg2 = image->image_size;
}

void bl2_plat_flush_bl31_params(void)
{
	flush_dcache_range((unsigned long)&bl31_params_mem,
			   sizeof(bl2_to_bl31_params_mem_t));
}

void bl2_plat_get_bl33_meminfo(meminfo_t *bl33_meminfo)
{
	bl33_meminfo->total_base = DDR_BASE;
	bl33_meminfo->total_size = DDR_SIZE;
	bl33_meminfo->free_base  = DDR_BASE;
	bl33_meminfo->free_size  = DDR_SIZE;
}
#endif /* LOAD_IMAGE_V2 */

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	struct mmc_device_info info;

#if !POPLAR_RECOVERY
	dw_mmc_params_t params = EMMC_INIT_PARAMS(POPLAR_EMMC_DESC_BASE);
#endif

	console_init(PL011_UART0_BASE, PL011_UART0_CLK_IN_HZ, PL011_BAUDRATE);

	/* Enable arch timer */
	generic_delay_timer_init();

	bl2_tzram_layout = *mem_layout;

#if !POPLAR_RECOVERY
	/* SoC-specific emmc register are initialized/configured by bootrom */
	INFO("BL2: initializing emmc\n");
	info.mmc_dev_type = MMC_IS_EMMC;
	dw_mmc_init(&params, &info);
#endif

	plat_io_setup();
}

void bl2_plat_arch_setup(void)
{
	plat_configure_mmu_el1(bl2_tzram_layout.total_base,
			       bl2_tzram_layout.total_size,
			       BL2_RO_BASE,
			       BL2_RO_LIMIT,
			       BL2_COHERENT_RAM_BASE,
			       BL2_COHERENT_RAM_LIMIT);
}

void bl2_platform_setup(void)
{
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_POPLAR_NS_IMAGE_OFFSET;
#endif
}
