/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/pl011.h>
#include <drivers/arm/pl061_gpio.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/mmc.h>
#include <drivers/synopsys/dw_mmc.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include "hi3798cv200.h"
#include "plat_private.h"

/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo_t bl1_tzram_layout;
static meminfo_t bl2_tzram_layout;
static console_pl011_t console;

/*
 * Cannot use default weak implementation in bl1_main.c because BL1 RW data is
 * not at the top of the secure memory.
 */
int bl1_plat_handle_post_image_load(unsigned int image_id)
{
	image_desc_t *image_desc;
	entry_point_info_t *ep_info;

	if (image_id != BL2_IMAGE_ID)
		return 0;

	/* Get the image descriptor */
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);

	/* Get the entry point info */
	ep_info = &image_desc->ep_info;

	bl2_tzram_layout.total_base = BL2_BASE;
	bl2_tzram_layout.total_size = BL32_LIMIT - BL2_BASE;

	flush_dcache_range((uintptr_t)&bl2_tzram_layout, sizeof(meminfo_t));

	ep_info->args.arg1 = (uintptr_t)&bl2_tzram_layout;

	VERBOSE("BL1: BL2 memory layout address = %p\n",
		(void *)&bl2_tzram_layout);

	return 0;
}

void bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_pl011_register(PL011_UART0_BASE, PL011_UART0_CLK_IN_HZ,
			       PL011_BAUDRATE, &console);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL1_RW_BASE;
	bl1_tzram_layout.total_size = BL1_RW_SIZE;

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
#if !POPLAR_RECOVERY
	struct mmc_device_info info;
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
