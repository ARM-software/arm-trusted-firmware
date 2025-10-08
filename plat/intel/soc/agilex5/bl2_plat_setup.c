/*
 * Copyright (c) 2019-2025, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
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

#include "agilex5_clock_manager.h"
#include "agilex5_ddr.h"
#include "agilex5_memory_controller.h"
#include "agilex5_mmc.h"
#include "agilex5_pinmux.h"
#include "agilex5_power_manager.h"
#include "agilex5_system_manager.h"
#include "ccu/ncore_ccu.h"
#include "combophy/combophy.h"
#include "nand/nand.h"
#include "qspi/cadence_qspi.h"
#include "sdmmc/sdmmc.h"
/* TODO: DTB not available */
// #include "socfpga_dt.h"
#include "socfpga_emac.h"
#include "socfpga_f2sdram_manager.h"
#include "socfpga_handoff.h"
#include "socfpga_mailbox.h"
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
	MAP_REGION_FLAT(DRAM_BASE, DRAM_SIZE,
		MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(PSS_BASE, PSS_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(MPFE_BASE, MPFE_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(OCRAM_BASE, OCRAM_SIZE,
		MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CCU_BASE, CCU_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MEM64_BASE, MEM64_SIZE,
		MT_DEVICE | MT_RW | MT_NS),
	MAP_REGION_FLAT(GIC_BASE, GIC_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	{0},
};

boot_source_type boot_source = BOOT_SOURCE;

void bl2_el3_early_platform_setup(u_register_t x0 __unused,
				  u_register_t x1 __unused,
				  u_register_t x2 __unused,
				  u_register_t x3 __unused)
{
	static console_t console;
	handoff reverse_handoff_ptr;
	uint32_t reg_val;

	/* Enable nonsecure access for peripherals and other misc components */
	enable_nonsecure_access();

	/* Bring all the required peripherals out of reset */
	deassert_peripheral_reset();

	/*
	 * Initialize the UART console early in BL2 EL3 boot flow to get
	 * the error/notice messages wherever required.
	 */
	console_16550_register(PLAT_INTEL_UART_BASE, PLAT_UART_CLOCK,
			       PLAT_BAUDRATE, &console);

	/* Generic delay timer init */
	generic_delay_timer_init();

	socfpga_delay_timer_init();

	/* Get the handoff data */
	if ((socfpga_get_handoff(&reverse_handoff_ptr)) != 0) {
		ERROR("SOCFPGA: Failed to get the correct handoff data\n");
		panic();
	}

	/* Configure the pinmux */
	config_pinmux(&reverse_handoff_ptr);

	/* Configure OCRAM to NON SECURE ACCESS */
	mmio_write_32(OCRAM_REGION_0_REG_BASE, OCRAM_NON_SECURE_ENABLE);
	mmio_write_32(SOCFPGA_L4_PER_SCR_REG_BASE + SOCFPGA_SDMMC_SECU_BIT,
		SOCFPGA_SDMMC_SECU_BIT_ENABLE);
	mmio_write_32(SOCFPGA_L4_SYS_SCR_REG_BASE + SOCFPGA_SDMMC_SECU_BIT,
		SOCFPGA_SDMMC_SECU_BIT_ENABLE);
	mmio_write_32(SOCFPGA_LWSOC2FPGA_SCR_REG_BASE,
		SOCFPGA_LWSOC2FPGA_ENABLE);

	/* Configure the clock manager */
	if ((config_clkmgr_handoff(&reverse_handoff_ptr)) != 0) {
		ERROR("SOCFPGA: Failed to initialize the clock manager\n");
		panic();
	}

	/* Configure power manager PSS SRAM power gate */
	config_pwrmgr_handoff(&reverse_handoff_ptr);

	/* Initialize the mailbox to enable communication between HPS and SDM */
	mailbox_init();

	/* Perform a handshake with certain peripherals before issuing a reset */
	config_hps_hs_before_warm_reset();

	/* TODO: watchdog init */
	//watchdog_init(clkmgr_get_rate(CLKMGR_WDT_CLK_ID));

	/* Initialize the CCU module for hardware cache coherency */
	init_ncore_ccu();

	socfpga_emac_init();

	/* DDR and IOSSM driver init */
	if ((agilex5_ddr_init(&reverse_handoff_ptr)) != 0) {
		ERROR("SOCFPGA: Failed to initialize the ddr.\n");
		panic();
	}

	/* TODO: DTB not available */
	// if (socfpga_dt_open_and_check(SOCFPGA_DTB_BASE, DT_COMPATIBLE_STR) < 0) {
		// ERROR("SOCFPGA: Failed to open device tree\n");
		// panic();
	// }

	if (combo_phy_init(&reverse_handoff_ptr) != 0) {
		ERROR("SOCFPGA: Combo Phy initialization failed\n");
	}

	/* Enable FPGA bridges as required */
	if (!intel_mailbox_is_fpga_not_ready()) {
		socfpga_bridges_enable(SOC2FPGA_MASK | LWHPS2FPGA_MASK |
				       FPGA2SOC_MASK | F2SDRAM0_MASK);
	}

	/* Configure USB 3.1 in system manager */
	reg_val = mmio_read_32(SOCFPGA_SYSMGR(USB3_MISC_CTRL_REG0));
	reg_val |= SYSMGR_USB3_MISC0_PORT_OVR_CURR_PIPE_PWR; /* set pipe power present bit */
	mmio_write_32(SOCFPGA_SYSMGR(USB3_MISC_CTRL_REG0), reg_val);
	VERBOSE("USB3_MISC_CTRL_REG0 = 0x%X\n", mmio_read_32(SOCFPGA_SYSMGR(USB3_MISC_CTRL_REG0)));
}

void bl2_el3_plat_arch_setup(void)
{
	unsigned long offset = 0;

	struct cdns_sdmmc_params params = EMMC_INIT_PARAMS((uintptr_t) &cdns_desc,
							   SDEMMC_SDCLK);

	params.sdmclk = clkmgr_get_rate(CLKMGR_SDMMC_CLK_ID);
	mmc_info.mmc_dev_type = MMC_DEVICE_TYPE;
	mmc_info.ocr_voltage = OCR_3_3_3_4 | OCR_3_2_3_3;

	INFO("SDMMC/NAND clock is %u\n", clkmgr_get_rate(CLKMGR_SDMMC_CLK_ID));

	/* Request ownership and direct access to QSPI */
	mailbox_hps_qspi_enable();

	switch (boot_source) {
	case BOOT_SOURCE_SDMMC:
		NOTICE("SOCFPGA: SDMMC boot\n");
		cdns_mmc_init(&params, &mmc_info);
		socfpga_io_setup(boot_source, PLAT_SDMMC_DATA_BASE);
		break;

	case BOOT_SOURCE_QSPI:
		NOTICE("SOCFPGA: QSPI boot\n");
		cad_qspi_init(0, QSPI_CONFIG_CPHA, QSPI_CONFIG_CPOL,
			QSPI_CONFIG_CSDA, QSPI_CONFIG_CSDADS,
			QSPI_CONFIG_CSEOT, QSPI_CONFIG_CSSOT, 0);
		if (ros_qspi_get_ssbl_offset(&offset) != ROS_RET_OK) {
			offset = PLAT_QSPI_DATA_BASE;
		}
		socfpga_io_setup(boot_source, offset);
		break;

	case BOOT_SOURCE_NAND:
		NOTICE("SOCFPGA: NAND boot\n");
		nand_init();
		socfpga_io_setup(boot_source, PLAT_NAND_DATA_BASE);
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
