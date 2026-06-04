/*
 * Copyright (c) 2015-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#if MEASURED_BOOT
#include "./include/rpi3_measured_boot.h"
#endif

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <lib/optee_utils.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/rpi3/gpio/rpi3_gpio.h>
#if TRANSFER_LIST
#include <tpm_event_log.h>
#include <transfer_list.h>
#endif /* TRANSFER_LIST */

#include <drivers/rpi3/sdhost/rpi3_sdhost.h>
#include <platform_def.h>
#include <rpi_shared.h>

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);
struct transfer_list_header __maybe_unused *bl2_tl;

/* Data structure which holds the MMC info */
static struct mmc_device_info mmc_info;

/* Variables that hold the eventlog addr and size for use in BL2 Measured Boot */
static uint8_t *event_log_start;
static size_t event_log_size;

static void rpi3_sdhost_setup(void)
{
	struct rpi3_sdhost_params params;

	memset(&params, 0, sizeof(struct rpi3_sdhost_params));
	params.reg_base = RPI3_SDHOST_BASE;
	params.bus_width = MMC_BUS_WIDTH_1;
	params.clk_rate = 50000000;
	params.clk_rate_initial = (RPI3_SDHOST_MAX_CLOCK / HC_CLOCKDIVISOR_MAXVAL);
	mmc_info.mmc_dev_type = MMC_IS_SD_HC;
	mmc_info.ocr_voltage = OCR_3_2_3_3 | OCR_3_3_3_4;
	rpi3_sdhost_init(&params, &mmc_info);
}

void rpi3_mboot_fetch_eventlog_info(uint8_t **eventlog_addr, size_t *eventlog_size)
{
	*eventlog_addr = event_log_start;
	*eventlog_size = event_log_size;
}

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe location before its reclaimed by later BL2 functionality.
 ******************************************************************************/

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			       u_register_t arg2, u_register_t arg3)
{
	meminfo_t *mem_layout = (meminfo_t *) arg1;

	/* Initialize the console to provide early debug support */
	rpi3_console_init();

	/* Enable arch timer */
	generic_delay_timer_init();

	/* Setup GPIO driver */
	rpi3_gpio_init();

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	/* Setup SDHost driver */
	rpi3_sdhost_setup();
	/* When TRANSFER_LIST is used, BL1 already set handoff args:
	 * arg3 = transfer list header; event log is inside TL.
	 * When legacy path, arg2/arg3 carry event log base/size.
	 */
#if TRANSFER_LIST
	bl2_tl = (struct transfer_list_header *)(uintptr_t)arg3;
	if (bl2_tl != NULL &&
	    transfer_list_check_header(bl2_tl) != TL_OPS_NON) {
		INFO("BL2: Transfer List found\n");
	} else {
		WARN("BL2: TransferList not found; reinit TL\n");
		bl2_tl = transfer_list_init((void *)(uintptr_t)FW_HANDOFF_BASE,
					    FW_HANDOFF_SIZE);
		if (!bl2_tl) {
			ERROR("BL2: Failed to re-initialize transfer list\n");
			panic();
		}
	}
#else
	event_log_start = (uint8_t *)(uintptr_t)arg2;
	event_log_size = arg3;
#endif

	plat_rpi3_io_setup();
}

void bl2_platform_setup(void)
{
	/*
	 * This is where a TrustZone address space controller and other
	 * security related peripherals would be configured.
	 */

	// Setup TOS Firmware Image Here - If I set it up earlier it get overwritten by event log
#if defined(SPD_spmd) && defined(PLAT_RPI3_SPMC_SP_MANIFEST_SIZE)
	bl_mem_params_node_t *next_param_node =
		get_bl_mem_params_node(TOS_FW_CONFIG_ID);
	assert(next_param_node != NULL);

	struct transfer_list_entry *te = transfer_list_add(bl2_tl, TL_TAG_DT_SPMC_MANIFEST,
			PLAT_RPI3_SPMC_SP_MANIFEST_SIZE, NULL);
	assert(te != NULL);

	next_param_node->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
	next_param_node->image_info.image_max_size = PLAT_RPI3_SPMC_SP_MANIFEST_SIZE;
	next_param_node->image_info.image_base =
		(uintptr_t)transfer_list_entry_data(te);
#endif /* defined(SPD_spmd) && defined(PLAT_RPI3_SPMC_SP_MANIFEST_SIZE) */
}

void rpi3_bl2_sync_transfer_list(void)
{
#if TRANSFER_LIST
	transfer_list_update_checksum(bl2_tl);
#endif
}
/*******************************************************************************
 * Perform the very early platform specific architectural setup here.
 ******************************************************************************/
void bl2_plat_arch_setup(void)
{
	rpi3_setup_page_tables(bl2_tzram_layout.total_base,
			       bl2_tzram_layout.total_size,
			       BL_CODE_BASE, BL_CODE_END,
			       BL_RO_DATA_BASE, BL_RO_DATA_END
#if USE_COHERENT_MEM
			       , BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END
#endif
			      );

	enable_mmu_el1(0);
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
#if defined(SPD_opteed) || defined(SPD_spmd)
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif /* defined(SPD_opteed) || defined(SPD_spmd) */
#if TRANSFER_LIST
	struct transfer_list_header *ns_tl = NULL;
#endif
	assert(bl_mem_params != NULL);

	switch (image_id) {
#if TRANSFER_LIST
	case TOS_FW_CONFIG_ID:
		/*
		 * Refresh the now stale checksum following loading of
		 * HW_CONFIG or TOS_FW_CONFIG into the TL.
		 */
		transfer_list_update_checksum(bl2_tl);
		break;
	case BL31_IMAGE_ID:
		/*
		 * arg0 is a bl_params_t reserved for bl31_early_platform_setup2
		 * we just need arg1 and arg3 for BL31 to update the TL from S
		 * to NS memory before it exits
		 */
		if (GET_RW(bl_mem_params->ep_info.spsr) == MODE_RW_64) {
			bl_mem_params->ep_info.args.arg1 =
				TRANSFER_LIST_HANDOFF_X1_VALUE(REGISTER_CONVENTION_VERSION);
		}
		bl_mem_params->ep_info.args.arg3 = (uintptr_t)bl2_tl;
		break;
#endif
	case BL32_IMAGE_ID:
#if defined(SPD_opteed) || defined(SPD_spmd)
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
				&pager_mem_params->image_info,
				&paged_mem_params->image_info);
		if (err != 0)
			WARN("OPTEE header parse error.\n");
#endif /* defined(SPD_opteed) || defined(SPD_spmd) */
		bl_mem_params->ep_info.spsr = rpi3_get_spsr_for_bl32_entry();
#if defined(SPD_spmd)
		bl_mem_params->ep_info.args.arg3 = (uintptr_t)bl2_tl;
#endif /* defined(SPD_spmd) */
		break;

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */

		bl_mem_params->ep_info.spsr = rpi3_get_spsr_for_bl33_entry();

#if TRANSFER_LIST
		if (bl2_tl) {
#ifdef FW_NS_HANDOFF_BASE
			/* relocate the tl to pre-allocate NS memory if macro provided */
			ns_tl = transfer_list_relocate(
				bl2_tl, (void *)(uintptr_t)FW_NS_HANDOFF_BASE,
				bl2_tl->max_size);
			if (!ns_tl) {
				ERROR("Relocate TL to 0x%lx failed\n",
				      (unsigned long)FW_NS_HANDOFF_BASE);
				return -1;
			}
			INFO("TL relocated to ns region\n");
#endif
		}

		if (!transfer_list_set_handoff_args(ns_tl,
						    &bl_mem_params->ep_info)) {
			WARN("Invalid TL, fallback to default arguments\n");
			bl_mem_params->ep_info.args.arg0 = 0xffff &
							   read_mpidr();
		}
#else
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
#endif

		/* Shutting down the SDHost driver to let BL33 drives SDHost.*/
		rpi3_sdhost_stop();
		break;

	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}
