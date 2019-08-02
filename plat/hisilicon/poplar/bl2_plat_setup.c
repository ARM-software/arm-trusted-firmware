/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/pl011.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/partition/partition.h>
#include <drivers/synopsys/dw_mmc.h>
#include <drivers/mmc.h>
#include <lib/mmio.h>
#include <lib/optee_utils.h>
#include <plat/common/platform.h>

#include "hi3798cv200.h"
#include "plat_private.h"

static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);
static console_pl011_t console;

/*******************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 ******************************************************************************/
int plat_poplar_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
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
#ifdef __aarch64__
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
#endif /* __aarch64__ */

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
#ifdef __aarch64__
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

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			       u_register_t arg2, u_register_t arg3)
{
	struct meminfo *mem_layout = (struct meminfo *)arg1;
#if !POPLAR_RECOVERY
	struct mmc_device_info info;

	dw_mmc_params_t params = EMMC_INIT_PARAMS(POPLAR_EMMC_DESC_BASE);
#endif

	console_pl011_register(PL011_UART0_BASE, PL011_UART0_CLK_IN_HZ,
			       PL011_BAUDRATE, &console);

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
			       BL_CODE_BASE,
			       BL_CODE_END,
			       BL_COHERENT_RAM_BASE,
			       BL_COHERENT_RAM_END);
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
