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
#include <dw_mmc.h>
#include <emmc.h>
#include <errno.h>
#include <hi6220.h>
#include <hisi_mcu.h>
#include <hisi_sram_map.h>
#include <mmio.h>
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

void bl2_plat_get_scp_bl2_meminfo(meminfo_t *scp_bl2_meminfo)
{
	scp_bl2_meminfo->total_base = SCP_BL2_BASE;
	scp_bl2_meminfo->total_size = SCP_BL2_SIZE;
	scp_bl2_meminfo->free_base = SCP_BL2_BASE;
	scp_bl2_meminfo->free_size = SCP_BL2_SIZE;
}

int bl2_plat_handle_scp_bl2(struct image_info *scp_bl2_image_info)
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
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);

	/* Fill BL3-2 related information if it exists */
#if BL32_BASE
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
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);

	return bl2_to_bl31_params;
}

struct entry_point_info *bl2_plat_get_bl31_ep_info(void)
{
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
 * On Hikey we only set the security state of the entrypoint
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
	unsigned long el_status;
	unsigned int mode;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	if (el_status)
		mode = MODE_EL2;
	else
		mode = MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	bl33_ep_info->spsr = SPSR_64(mode, MODE_SP_ELX,
				       DISABLE_ALL_EXCEPTIONS);
	SET_SECURITY_STATE(bl33_ep_info->h.attr, NON_SECURE);
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
	bl33_meminfo->free_base = DDR_BASE;
	bl33_meminfo->free_size = DDR_SIZE;
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
}
