/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <boot_api.h>
#include <console.h>
#include <debug.h>
#include <delay_timer.h>
#include <desc_image_load.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <stm32mp1_private.h>
#include <stm32mp1_pwr.h>
#include <stm32mp1_rcc.h>
#include <string.h>
#include <xlat_tables_v2.h>

void bl2_el3_early_platform_setup(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	stm32mp1_save_boot_ctx_address(arg0);
}

void bl2_platform_setup(void)
{
	INFO("BL2 runs SP_MIN setup\n");
}

void bl2_el3_plat_arch_setup(void)
{
	/*
	 * Disable the backup domain write protection.
	 * The protection is enable at each reset by hardware
	 * and must be disabled by software.
	 */
	mmio_setbits_32(PWR_BASE + PWR_CR1, PWR_CR1_DBP);

	while ((mmio_read_32(PWR_BASE + PWR_CR1) & PWR_CR1_DBP) == 0U) {
		;
	}

	/* Reset backup domain on cold boot cases */
	if ((mmio_read_32(RCC_BASE + RCC_BDCR) & RCC_BDCR_RTCSRC_MASK) == 0U) {
		mmio_setbits_32(RCC_BASE + RCC_BDCR, RCC_BDCR_VSWRST);

		while ((mmio_read_32(RCC_BASE + RCC_BDCR) & RCC_BDCR_VSWRST) ==
		       0U) {
			;
		}

		mmio_clrbits_32(RCC_BASE + RCC_BDCR, RCC_BDCR_VSWRST);
	}

	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	/* Prevent corruption of preloaded BL32 */
	mmap_add_region(BL32_BASE, BL32_BASE,
			BL32_LIMIT - BL32_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);

	/* Prevent corruption of preloaded Device Tree */
	mmap_add_region(DTB_BASE, DTB_BASE,
			DTB_LIMIT - DTB_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);

	configure_mmu();

	generic_delay_timer_init();

	stm32mp1_io_setup();
}
