/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2020, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/synopsys/dw_mmc.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/xlat_tables/xlat_tables.h>

#include "qspi/cadence_qspi.h"
#include "socfpga_emac.h"
#include "socfpga_handoff.h"
#include "socfpga_mailbox.h"
#include "socfpga_private.h"
#include "socfpga_reset_manager.h"
#include "socfpga_system_manager.h"
#include "s10_clock_manager.h"
#include "s10_memory_controller.h"
#include "s10_pinmux.h"
#include "wdt/watchdog.h"


const mmap_region_t plat_stratix10_mmap[] = {
	MAP_REGION_FLAT(DRAM_BASE, DRAM_SIZE,
		MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE2_BASE, DEVICE2_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(OCRAM_BASE, OCRAM_SIZE,
		MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE3_BASE, DEVICE3_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MEM64_BASE, MEM64_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(DEVICE4_BASE, DEVICE4_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	{0},
};

boot_source_type boot_source = BOOT_SOURCE;

void bl2_el3_early_platform_setup(u_register_t x0, u_register_t x1,
				u_register_t x2, u_register_t x4)
{
	static console_t console;
	handoff reverse_handoff_ptr;

	generic_delay_timer_init();

	if (socfpga_get_handoff(&reverse_handoff_ptr))
		return;
	config_pinmux(&reverse_handoff_ptr);

	config_clkmgr_handoff(&reverse_handoff_ptr);
	enable_nonsecure_access();
	deassert_peripheral_reset();
	config_hps_hs_before_warm_reset();

	watchdog_init(get_wdt_clk());

	console_16550_register(PLAT_UART0_BASE, get_uart_clk(), PLAT_BAUDRATE,
		&console);

	socfpga_emac_init();
	socfpga_delay_timer_init();
	init_hard_memory_controller();
	mailbox_init();

	if (!intel_mailbox_is_fpga_not_ready())
		socfpga_bridges_enable();
}


void bl2_el3_plat_arch_setup(void)
{

	struct mmc_device_info info;
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL2_BASE, BL2_END - BL2_BASE,
			MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL_CODE_BASE, BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE,
			BL_RO_DATA_END - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE),
#if USE_COHERENT_MEM_BAR
		MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE),
#endif
		{0},
	};

	setup_page_tables(bl_regions, plat_stratix10_mmap);

	enable_mmu_el3(0);

	dw_mmc_params_t params = EMMC_INIT_PARAMS(0x100000, get_mmc_clk());

	info.mmc_dev_type = MMC_IS_SD;
	info.ocr_voltage = OCR_3_3_3_4 | OCR_3_2_3_3;

	switch (boot_source) {
	case BOOT_SOURCE_SDMMC:
		dw_mmc_init(&params, &info);
		socfpga_io_setup(boot_source);
		break;

	case BOOT_SOURCE_QSPI:
		mailbox_set_qspi_open();
		mailbox_set_qspi_direct();
		cad_qspi_init(0, QSPI_CONFIG_CPHA, QSPI_CONFIG_CPOL,
			QSPI_CONFIG_CSDA, QSPI_CONFIG_CSDADS,
			QSPI_CONFIG_CSEOT, QSPI_CONFIG_CSSOT, 0);
		socfpga_io_setup(boot_source);
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

