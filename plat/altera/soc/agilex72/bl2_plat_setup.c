/*
 * Copyright (c) 2019-2026, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/cadence/cdns_sdmmc.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/synopsys/dw_mmc.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "agilex72_clock_manager.h"
#include "agilex72_ddr.h"
#include "agilex72_memory_controller.h"
#include "agilex72_pinmux.h"
#include "agilex72_power_manager.h"
#include "agilex72_system_manager.h"
#include "ccu/ncore_ccu.h"
#include "combophy/combophy.h"
#include "mailbox/socfpga_mailbox_core.h"
#include "nand/nand.h"
#include "sdmmc/sdmmc.h"
#include "socfpga_emac.h"
#include "socfpga_f2sdram_manager.h"
#include "socfpga_handoff.h"
#include "socfpga_private.h"
#include "socfpga_reset_manager.h"
#include "socfpga_ros.h"
#include "socfpga_vab.h"
#include "wdt/watchdog.h"

/* Declare mmc_info */
static struct mmc_device_info mmc_info;

/* Declare cadence idmac descriptor */
extern struct cdns_idmac_desc cdns_desc[CONFIG_CDNS_DESC_COUNT] __aligned(8);

const mmap_region_t agilex_plat_mmap[] = {
	MAP_REGION_FLAT(DRAM_BASE, DRAM_SIZE, MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(PSS_BASE, PSS_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(MPFE_BASE, MPFE_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(OCRAM_BASE, OCRAM_SIZE,
			MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CCU_BASE, CCU_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MEM64_BASE, MEM64_SIZE, MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(GIC_BASE, GIC_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{0},
};

boot_source_type boot_source = BOOT_SOURCE;

void bl2_early_platform_setup2(u_register_t x0 __unused,
			       u_register_t x1 __unused,
			       u_register_t x2 __unused,
			       u_register_t x3 __unused)
{
	static console_t console;
	uint32_t reg_val;

	/*
	 * Initialize the UART console early in BL2 EL3 boot flow to get
	 * the error/notice messages wherever required.
	 */
	console_16550_register(PLAT_INTEL_UART_BASE, PLAT_UART_CLOCK,
			       PLAT_BAUDRATE, &console);

	/* Generic delay timer init */
	generic_delay_timer_init();

	/*
	 * Configure the clock manager. The routine performs the GPPLL/clock-
	 * manager bring-up itself (see config_clkmgr_handoff() in
	 * agilex72_clock_manager.c) and does not consume handoff data, so NULL
	 * is passed.
	 */
	if ((config_clkmgr_handoff(NULL)) != 0) {
		ERROR("SOCFPGA: Failed to initialize the clock manager\n");
		panic();
	}

	/* Initialize the mailbox to enable communication between HPS and SDM */
	int status;

	status = mailbox_core_init(MAILBOX_BACKEND_MCSR);
	if (status != 0) {
		ERROR("MCSR mailbox init failed: %d\n", status);
		panic();
	}
	NOTICE("BL2: MCSR mailbox selected.\n");

	/* Configure USB 3.1 in system manager */
	reg_val = mmio_read_32(SOCFPGA_SYSMGR(USB3_MISC_CTRL_REG0));
	reg_val |= SYSMGR_USB3_MISC0_PORT_OVR_CURR_PIPE_PWR; /* set pipe power present bit */
	mmio_write_32(SOCFPGA_SYSMGR(USB3_MISC_CTRL_REG0), reg_val);
	VERBOSE("USB3_MISC_CTRL_REG0 = 0x%X\n", mmio_read_32(SOCFPGA_SYSMGR(USB3_MISC_CTRL_REG0)));
}

void bl2_plat_arch_setup(void)
{

	struct cdns_sdmmc_params params = EMMC_INIT_PARAMS((uintptr_t)&cdns_desc,
							   SDEMMC_SDCLK);

	params.sdmclk = clkmgr_get_rate(CLKMGR_SDMMC_CLK_ID);
#if AGILEX72_SIMICS_BYPASS
	/*
	 * Simics km-hps-sdmmc does not model combo-PHY HRS04/HRS05 readback or
	 * the SRS09 card-detect line. Advertise runtime quirks so the shared
	 * Cadence driver skips those steps. This does not skip FIP loading —
	 * front-door boot still reads the FIP from SD.
	 */
	params.quirks |= CDNS_SDMMC_QUIRK_NO_PHY_INIT |
			 CDNS_SDMMC_QUIRK_NO_CARD_DETECT;
#endif
	mmc_info.mmc_dev_type = MMC_DEVICE_TYPE;
	mmc_info.ocr_voltage = OCR_3_3_3_4 | OCR_3_2_3_3;

	NOTICE("SDMMC/NAND clock is %u\n", clkmgr_get_rate(CLKMGR_SDMMC_CLK_ID));

	/* Request ownership and direct access to QSPI */
	mailbox_hps_qspi_enable();

	switch (boot_source) {
	case BOOT_SOURCE_SDMMC:
		NOTICE("SOCFPGA: SDMMC boot\n");
		cdns_mmc_init(&params, &mmc_info);
		socfpga_io_setup(boot_source, PLAT_SDMMC_DATA_BASE);
		break;
	/*
	 * In Agilex72, both QSPI (x4) and OSPI (x8) flash are accessed through the
	 * same Cadence xSPI hardware controller
	 */
	case BOOT_SOURCE_QSPI:
	case BOOT_SOURCE_OSPI:
		/* xSPI driver enabled in a later commit */
		ERROR("SOCFPGA: xSPI boot not enabled in this commit\n");
		panic();
		break;

	default:
		ERROR("SOCFPGA: Unsupported boot source\n");
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

	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params);

#if SOCFPGA_SECURE_VAB_AUTH
	/*
	 * VAB Authentication start here.
	 * If failed to authenticate, shall not proceed to process BL31 and hang.
	 */
	int ret = 0;

	ret = socfpga_vab_init(image_id);
	if (ret < 0) {
		ERROR("SOCFPGA: VAB Authentication failed\n");
		wfi();
	}
#endif

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
