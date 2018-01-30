/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
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
#include <emmc.h>
#include <errno.h>
#include <hi6220.h>
#include <hisi_mcu.h>
#include <hisi_sram_map.h>
#include <mmio.h>
#ifdef SPD_opteed
#include <optee_utils.h>
#endif
#include <platform_def.h>
#include <sp804_delay_timer.h>
#include <string.h>

#include "hikey_def.h"
#include "hikey_private.h"

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL2_RO_BASE (unsigned long)(&__RO_START__)
#define BL2_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL2_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL2_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/*******************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 ******************************************************************************/
int plat_hikey_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	/* Enable MCU SRAM */
	hisi_mcu_enable_sram();

	/* Load MCU binary into SRAM */
	hisi_mcu_load_image(scp_bl2_image_info->image_base,
			    scp_bl2_image_info->image_size);
	/* Let MCU running */
	hisi_mcu_start_run();

	INFO("%s: MCU PC is at 0x%x\n",
	     __func__, mmio_read_32(AO_SC_MCU_SUBSYS_STAT2));
	INFO("%s: AO_SC_PERIPH_CLKSTAT4 is 0x%x\n",
	     __func__, mmio_read_32(AO_SC_PERIPH_CLKSTAT4));
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t hikey_get_spsr_for_bl32_entry(void)
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
uint32_t hikey_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = EL_IMPLEMENTED(2) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#else
uint32_t hikey_get_spsr_for_bl33_entry(void)
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

int hikey_bl2_handle_post_image_load(unsigned int image_id)
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
#endif
		bl_mem_params->ep_info.spsr = hikey_get_spsr_for_bl32_entry();
		break;
#endif

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = hikey_get_spsr_for_bl33_entry();
		break;

#ifdef SCP_BL2_BASE
	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific */
		err = plat_hikey_bl2_handle_scp_bl2(&bl_mem_params->image_info);
		if (err) {
			WARN("Failure in platform-specific handling of SCP_BL2 image.\n");
		}
		break;
#endif
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return hikey_bl2_handle_post_image_load(image_id);
}

static void reset_dwmmc_clk(void)
{
	unsigned int data;

	/* disable mmc0 bus clock */
	mmio_write_32(PERI_SC_PERIPH_CLKDIS0, PERI_CLK0_MMC0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (data & PERI_CLK0_MMC0);
	/* enable mmc0 bus clock */
	mmio_write_32(PERI_SC_PERIPH_CLKEN0, PERI_CLK0_MMC0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while (!(data & PERI_CLK0_MMC0));
	/* reset mmc0 clock domain */
	mmio_write_32(PERI_SC_PERIPH_RSTEN0, PERI_RST0_MMC0);

	/* bypass mmc0 clock phase */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL2);
	data |= 3;
	mmio_write_32(PERI_SC_PERIPH_CTRL2, data);

	/* disable low power */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL13);
	data |= 1 << 3;
	mmio_write_32(PERI_SC_PERIPH_CTRL13, data);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
	} while (!(data & PERI_RST0_MMC0));

	/* unreset mmc0 clock domain */
	mmio_write_32(PERI_SC_PERIPH_RSTDIS0, PERI_RST0_MMC0);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
	} while (data & PERI_RST0_MMC0);
}

static void hikey_boardid_init(void)
{
	u_register_t midr;

	midr = read_midr();
	mmio_write_32(MEMORY_AXI_CHIP_ADDR, midr);
	INFO("[BDID] [%x] midr: 0x%x\n", MEMORY_AXI_CHIP_ADDR,
	     (unsigned int)midr);

	mmio_write_32(MEMORY_AXI_BOARD_TYPE_ADDR, 0);
	mmio_write_32(MEMORY_AXI_BOARD_ID_ADDR, 0x2b);

	mmio_write_32(ACPU_ARM64_FLAGA, 0x1234);
	mmio_write_32(ACPU_ARM64_FLAGB, 0x5678);
}

static void hikey_sd_init(void)
{
	/* switch pinmux to SD */
	mmio_write_32(IOMG_SD_CLK, IOMG_MUX_FUNC0);
	mmio_write_32(IOMG_SD_CMD, IOMG_MUX_FUNC0);
	mmio_write_32(IOMG_SD_DATA0, IOMG_MUX_FUNC0);
	mmio_write_32(IOMG_SD_DATA1, IOMG_MUX_FUNC0);
	mmio_write_32(IOMG_SD_DATA2, IOMG_MUX_FUNC0);
	mmio_write_32(IOMG_SD_DATA3, IOMG_MUX_FUNC0);

	mmio_write_32(IOCG_SD_CLK, IOCG_INPUT_16MA);
	mmio_write_32(IOCG_SD_CMD, IOCG_INPUT_12MA);
	mmio_write_32(IOCG_SD_DATA0, IOCG_INPUT_12MA);
	mmio_write_32(IOCG_SD_DATA1, IOCG_INPUT_12MA);
	mmio_write_32(IOCG_SD_DATA2, IOCG_INPUT_12MA);
	mmio_write_32(IOCG_SD_DATA3, IOCG_INPUT_12MA);

	/* set SD Card detect as nopull */
	mmio_write_32(IOCG_GPIO8, 0);
}

static void hikey_jumper_init(void)
{
	/* set jumper detect as nopull */
	mmio_write_32(IOCG_GPIO24, 0);
	/* set jumper detect as GPIO */
	mmio_write_32(IOMG_GPIO24, IOMG_MUX_FUNC0);
}

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	dw_mmc_params_t params;

	/* Initialize the console to provide early debug support */
	console_init(CONSOLE_BASE, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	/* Clear SRAM since it'll be used by MCU right now. */
	memset((void *)SRAM_BASE, 0, SRAM_SIZE);
	clean_dcache_range(SRAM_BASE, SRAM_SIZE);

	sp804_timer_init(SP804_TIMER0_BASE, 10, 192);
	dsb();
	hikey_ddr_init();

	hikey_boardid_init();
	init_acpu_dvfs();
	hikey_sd_init();
	hikey_jumper_init();

	reset_dwmmc_clk();
	memset(&params, 0, sizeof(dw_mmc_params_t));
	params.reg_base = DWMMC0_BASE;
	params.desc_base = HIKEY_MMC_DESC_BASE;
	params.desc_size = 1 << 20;
	params.clk_rate = 24 * 1000 * 1000;
	params.bus_width = EMMC_BUS_WIDTH_8;
	params.flags = EMMC_FLAG_CMD23;
	dw_mmc_init(&params);

	hikey_io_setup();
}

void bl2_plat_arch_setup(void)
{
	hikey_init_mmu_el1(bl2_tzram_layout.total_base,
			   bl2_tzram_layout.total_size,
			   BL2_RO_BASE,
			   BL2_RO_LIMIT,
			   BL2_COHERENT_RAM_BASE,
			   BL2_COHERENT_RAM_LIMIT);
}

void bl2_platform_setup(void)
{
	hikey_security_setup();
}
