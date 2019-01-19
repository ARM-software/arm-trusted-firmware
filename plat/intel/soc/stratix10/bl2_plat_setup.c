/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <gicv2.h>

#include <generic_delay_timer.h>
#include <console.h>
#include <bl_common.h>
#include <debug.h>
#include <desc_image_load.h>
#include <errno.h>
#include <io/io_storage.h>
#include <image_decompress.h>
#include <platform.h>
#include <platform_def.h>
#include <platform_private.h>
#include <dw_mmc.h>
#include <mmio.h>

#include "s10_memory_controller.h"
#include "s10_reset_manager.h"
#include "s10_clock_manager.h"
#include "s10_handoff.h"
#include "s10_pinmux.h"
#include "aarch64/stratix10_private.h"

#define BL2_RO_BASE (unsigned long)(&__RO_START__)
#define BL2_RO_LIMIT (unsigned long)(&__RO_END__)

#define BL2_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL2_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

boot_source_type boot_source;

void bl2_el3_early_platform_setup(u_register_t x0, u_register_t x1,
				u_register_t x2, u_register_t x4)
{
	handoff reverse_handoff_ptr;

	generic_delay_timer_init();

	if (s10_get_handoff(&reverse_handoff_ptr))
		return;
	config_pinmux(&reverse_handoff_ptr);
	boot_source = reverse_handoff_ptr.boot_source;

	config_clkmgr_handoff(&reverse_handoff_ptr);
	enable_nonsecure_access();
	deassert_peripheral_reset();
	config_hps_hs_before_warm_reset();

	console_init(PLAT_UART0_BASE, PLAT_UART_CLOCK, PLAT_BAUDRATE);

	plat_delay_timer_init();
	init_hard_memory_controller();
}

void bl2_el3_plat_arch_setup(void)
{
	struct mmc_device_info info;
	dw_mmc_params_t params = EMMC_INIT_PARAMS(0x100000);

	info.mmc_dev_type = MMC_IS_SD;

	plat_configure_mmu_el3(BL2_RO_BASE,
			BL2_COHERENT_RAM_LIMIT - BL2_RO_BASE,
			BL2_RO_BASE, /* l-loader and BL1 ROM */
			BL2_RO_LIMIT,
#if USE_COHERENT_MEM
			BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END
#endif
	);
	switch (boot_source) {
	case BOOT_SOURCE_SDMMC:
		dw_mmc_init(&params, &info);
		stratix10_io_setup();
		break;
	default:
		ERROR("Unsupported boot source\n");
		panic();
		break;
	}
}

uint32_t get_spsr_for_bl33_entry(void)
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


int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	switch (image_id) {
	case BL33_IMAGE_ID:
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = get_spsr_for_bl33_entry();
		break;
	default:
		break;
	}

	return 0;
}

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl2_platform_setup(void)
{
}

