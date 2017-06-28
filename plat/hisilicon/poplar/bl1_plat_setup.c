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
#include <errno.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <pl061_gpio.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <tbbr_img_def.h>
#include "../../bl1/bl1_private.h"
#include "hi3798cv200.h"
#include "plat_private.h"

/* Symbols from link script for conherent section */
extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

#define BL1_COHERENT_RAM_BASE	(unsigned long)(&__COHERENT_RAM_START__)
#define BL1_COHERENT_RAM_LIMIT	(unsigned long)(&__COHERENT_RAM_END__)

/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo_t bl1_tzram_layout;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

void bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_init(PL011_UART0_BASE, PL011_UART0_CLK_IN_HZ, PL011_BAUDRATE);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL_MEM_BASE;
	bl1_tzram_layout.total_size = BL_MEM_SIZE;

	/* Calculate how much RAM BL1 is using and how much remains free */
	bl1_tzram_layout.free_base = BL_MEM_BASE;
	bl1_tzram_layout.free_size = BL_MEM_SIZE;

	reserve_mem(&bl1_tzram_layout.free_base,
		    &bl1_tzram_layout.free_size,
		    BL1_RAM_BASE,
		    BL1_RAM_LIMIT - BL1_RAM_BASE);

	INFO("BL1: 0x%lx - 0x%lx [size = %zu]\n", BL1_RAM_BASE, BL1_RAM_LIMIT,
	     BL1_RAM_LIMIT - BL1_RAM_BASE);
}

void bl1_plat_arch_setup(void)
{
	plat_configure_mmu_el3(bl1_tzram_layout.total_base,
			       bl1_tzram_layout.total_size,
			       BL_MEM_BASE, /* l-loader and BL1 ROM */
			       BL1_RO_LIMIT,
			       BL1_COHERENT_RAM_BASE,
			       BL1_COHERENT_RAM_LIMIT);
}

void bl1_platform_setup(void)
{
	int i;

	generic_delay_timer_init();

	pl061_gpio_init();
	for (i = 0; i < GPIO_MAX; i++)
		pl061_gpio_register(GPIO_BASE(i), i);

	plat_io_setup();
}

unsigned int bl1_plat_get_next_image_id(void)
{
	return BL2_IMAGE_ID;
}
