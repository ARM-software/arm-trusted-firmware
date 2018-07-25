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
#include <stm32mp1_clk.h>
#include <stm32mp1_dt.h>
#include <stm32mp1_pmic.h>
#include <stm32mp1_private.h>
#include <stm32mp1_context.h>
#include <stm32mp1_pwr.h>
#include <stm32mp1_ram.h>
#include <stm32mp1_rcc.h>
#include <stm32mp1_reset.h>
#include <string.h>
#include <xlat_tables_v2.h>

void bl2_el3_early_platform_setup(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	stm32mp1_save_boot_ctx_address(arg0);
}

void bl2_platform_setup(void)
{
	int ret;

	if (dt_check_pmic()) {
		initialize_pmic();
	}

	ret = stm32mp1_ddr_probe();
	if (ret < 0) {
		ERROR("Invalid DDR init: error %d\n", ret);
		panic();
	}

	INFO("BL2 runs SP_MIN setup\n");
}

void bl2_el3_plat_arch_setup(void)
{
	int32_t result;
	struct dt_node_info dt_dev_info;
	const char *board_model;
	boot_api_context_t *boot_context =
		(boot_api_context_t *)stm32mp1_get_boot_ctx_address();
	uint32_t clk_rate;

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

	if (dt_open_and_check() < 0) {
		panic();
	}

	if (stm32mp1_clk_probe() < 0) {
		panic();
	}

	if (stm32mp1_clk_init() < 0) {
		panic();
	}

	result = dt_get_stdout_uart_info(&dt_dev_info);

	if ((result <= 0) ||
	    (dt_dev_info.status == 0U) ||
	    (dt_dev_info.clock < 0) ||
	    (dt_dev_info.reset < 0)) {
		goto skip_console_init;
	}

	if (dt_set_stdout_pinctrl() != 0) {
		goto skip_console_init;
	}

	if (stm32mp1_clk_enable((unsigned long)dt_dev_info.clock) != 0) {
		goto skip_console_init;
	}

	stm32mp1_reset_assert((uint32_t)dt_dev_info.reset);
	udelay(2);
	stm32mp1_reset_deassert((uint32_t)dt_dev_info.reset);
	mdelay(1);

	clk_rate = stm32mp1_clk_get_rate((unsigned long)dt_dev_info.clock);

	if (console_init(dt_dev_info.base, clk_rate,
			 STM32MP1_UART_BAUDRATE) == 0) {
		panic();
	}

	board_model = dt_get_board_model();
	if (board_model != NULL) {
		NOTICE("%s\n", board_model);
	}

skip_console_init:

	if (stm32_save_boot_interface(boot_context->boot_interface_selected,
				      boot_context->boot_interface_instance) !=
	    0) {
		ERROR("Cannot save boot interface\n");
	}

	stm32mp1_arch_security_setup();

	stm32mp1_io_setup();
}
