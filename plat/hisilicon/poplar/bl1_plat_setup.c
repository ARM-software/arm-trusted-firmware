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
#include <generic_delay_timer.h>
#include <mmc.h>
#include <mmio.h>
#include <pl061_gpio.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <tbbr_img_def.h>
#include "../../bl1/bl1_private.h"
#include "hi3798cv200.h"
#include "plat_private.h"

/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo_t bl1_tzram_layout;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

#if LOAD_IMAGE_V2
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
	 * Cannot use default weak implementation in bl1main.c because
	 * BL1 RW data is not at the top of bl1_mem_layout
	 */
	bl2_mem_layout->total_base = BL2_BASE;
	bl2_mem_layout->total_size = BL32_LIMIT - BL2_BASE;

	flush_dcache_range((unsigned long)bl2_mem_layout, sizeof(meminfo_t));
}
#endif /* LOAD_IMAGE_V2 */

void bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_init(PL011_UART0_BASE, PL011_UART0_CLK_IN_HZ, PL011_BAUDRATE);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL1_RW_BASE;
	bl1_tzram_layout.total_size = BL1_RW_SIZE;

#if !LOAD_IMAGE_V2
	/* Calculate how much RAM BL1 is using and how much remains free */
	bl1_tzram_layout.free_base = BL1_RW_BASE;
	bl1_tzram_layout.free_size = BL1_RW_SIZE;

	reserve_mem(&bl1_tzram_layout.free_base,
		    &bl1_tzram_layout.free_size,
		    BL1_RAM_BASE,
		    BL1_RAM_LIMIT - BL1_RAM_BASE);
#endif

	INFO("BL1: 0x%lx - 0x%lx [size = %zu]\n", BL1_RAM_BASE, BL1_RAM_LIMIT,
	     BL1_RAM_LIMIT - BL1_RAM_BASE);
}

void bl1_plat_arch_setup(void)
{
	plat_configure_mmu_el3(bl1_tzram_layout.total_base,
			       bl1_tzram_layout.total_size,
			       BL1_RO_BASE, /* l-loader and BL1 ROM */
			       BL1_RO_LIMIT,
			       BL_COHERENT_RAM_BASE,
			       BL_COHERENT_RAM_END);
}

void bl1_platform_setup(void)
{
	int i;
	struct mmc_device_info info;
#if !POPLAR_RECOVERY
	dw_mmc_params_t params = EMMC_INIT_PARAMS(POPLAR_EMMC_DESC_BASE);
#endif

	generic_delay_timer_init();

	pl061_gpio_init();
	for (i = 0; i < GPIO_MAX; i++)
		pl061_gpio_register(GPIO_BASE(i), i);

#if !POPLAR_RECOVERY
	/* SoC-specific emmc register are initialized/configured by bootrom */
	INFO("BL1: initializing emmc\n");
	info.mmc_dev_type = MMC_IS_EMMC;
	dw_mmc_init(&params, &info);
#endif

	plat_io_setup();
}

unsigned int bl1_plat_get_next_image_id(void)
{
	return BL2_IMAGE_ID;
}
