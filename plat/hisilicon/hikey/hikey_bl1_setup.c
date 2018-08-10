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
#include <dw_mmc.h>
#include <errno.h>
#include <hi6220.h>
#include <hikey_def.h>
#include <hikey_layout.h>
#include <mmc.h>
#include <mmio.h>
#include <platform.h>
#include <string.h>
#include <tbbr/tbbr_img_desc.h>

#include "../../bl1/bl1_private.h"
#include "hikey_private.h"

/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo_t bl1_tzram_layout;

enum {
	BOOT_NORMAL = 0,
	BOOT_USB_DOWNLOAD,
	BOOT_UART_DOWNLOAD,
};

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*******************************************************************************
 * Function that takes a memory layout into which BL2 has been loaded and
 * populates a new memory layout for BL2 that ensures that BL1's data sections
 * resident in secure RAM are not visible to BL2.
 ******************************************************************************/
void bl1_init_bl2_mem_layout(const meminfo_t *bl1_mem_layout,
			     meminfo_t *bl2_mem_layout)
{

	assert(bl1_mem_layout != NULL);
	assert(bl2_mem_layout != NULL);

	/*
	 * Cannot remove BL1 RW data from the scope of memory visible to BL2
	 * like arm platforms because they overlap in hikey
	 */
	bl2_mem_layout->total_base = BL2_BASE;
	bl2_mem_layout->total_size = BL32_SRAM_LIMIT - BL2_BASE;

	flush_dcache_range((unsigned long)bl2_mem_layout, sizeof(meminfo_t));
}

/*
 * Perform any BL1 specific platform actions.
 */
void bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_init(CONSOLE_BASE, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL1_RW_BASE;
	bl1_tzram_layout.total_size = BL1_RW_SIZE;

	INFO("BL1: 0x%lx - 0x%lx [size = %lu]\n", BL1_RAM_BASE, BL1_RAM_LIMIT,
	     BL1_RAM_LIMIT - BL1_RAM_BASE); /* bl1_size */
}

/*
 * Perform the very early platform specific architecture setup here. At the
 * moment this only does basic initialization. Later architectural setup
 * (bl1_arch_setup()) does not do anything platform specific.
 */
void bl1_plat_arch_setup(void)
{
	hikey_init_mmu_el3(bl1_tzram_layout.total_base,
			   bl1_tzram_layout.total_size,
			   BL1_RO_BASE,
			   BL1_RO_LIMIT,
			   BL_COHERENT_RAM_BASE,
			   BL_COHERENT_RAM_END);
}

/*
 * Function which will perform any remaining platform-specific setup that can
 * occur after the MMU and data cache have been enabled.
 */
void bl1_platform_setup(void)
{
	dw_mmc_params_t params;
	struct mmc_device_info info;

	assert((HIKEY_BL1_MMC_DESC_BASE >= SRAM_BASE) &&
	       ((SRAM_BASE + SRAM_SIZE) >=
		(HIKEY_BL1_MMC_DATA_BASE + HIKEY_BL1_MMC_DATA_SIZE)));
	hikey_sp804_init();
	hikey_gpio_init();
	hikey_pmussi_init();
	hikey_hi6553_init();

	hikey_rtc_init();

	hikey_mmc_pll_init();

	memset(&params, 0, sizeof(dw_mmc_params_t));
	params.reg_base = DWMMC0_BASE;
	params.desc_base = HIKEY_BL1_MMC_DESC_BASE;
	params.desc_size = 1 << 20;
	params.clk_rate = 24 * 1000 * 1000;
	params.bus_width = MMC_BUS_WIDTH_8;
	params.flags = MMC_FLAG_CMD23;
	info.mmc_dev_type = MMC_IS_EMMC;
	dw_mmc_init(&params, &info);

	hikey_io_setup();
}

/*
 * The following function checks if Firmware update is needed,
 * by checking if TOC in FIP image is valid or not.
 */
unsigned int bl1_plat_get_next_image_id(void)
{
	int32_t boot_mode;
	unsigned int ret;

	boot_mode = mmio_read_32(ONCHIPROM_PARAM_BASE);
	switch (boot_mode) {
	case BOOT_USB_DOWNLOAD:
	case BOOT_UART_DOWNLOAD:
		ret = NS_BL1U_IMAGE_ID;
		break;
	default:
		WARN("Invalid boot mode is found:%d\n", boot_mode);
		panic();
	}
	return ret;
}

image_desc_t *bl1_plat_get_image_desc(unsigned int image_id)
{
	unsigned int index = 0;

	while (bl1_tbbr_image_descs[index].image_id != INVALID_IMAGE_ID) {
		if (bl1_tbbr_image_descs[index].image_id == image_id)
			return &bl1_tbbr_image_descs[index];

		index++;
	}

	return NULL;
}

void bl1_plat_set_ep_info(unsigned int image_id,
		entry_point_info_t *ep_info)
{
	uint64_t data = 0;

	if (image_id == BL2_IMAGE_ID)
		panic();
	inv_dcache_range(NS_BL1U_BASE, NS_BL1U_SIZE);
	__asm__ volatile ("mrs	%0, cpacr_el1" : "=r"(data));
	do {
		data |= 3 << 20;
		__asm__ volatile ("msr	cpacr_el1, %0" : : "r"(data));
		__asm__ volatile ("mrs	%0, cpacr_el1" : "=r"(data));
	} while ((data & (3 << 20)) != (3 << 20));
	INFO("cpacr_el1:0x%llx\n", data);

	ep_info->args.arg0 = 0xffff & read_mpidr();
	ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);
}
