/*
 * Copyright (c) 2023-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/clk.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <stm32mp_common.h>
#include <stm32mp_dt.h>

#define BOOT_CTX_ADDR	0x0e000020UL

static void print_reset_reason(void)
{
	uint32_t rstsr = mmio_read_32(stm32mp_rcc_base() + RCC_C1BOOTRSTSCLRR);

	if (rstsr == 0U) {
		WARN("Reset reason unknown\n");
		return;
	}

	INFO("Reset reason (0x%x):\n", rstsr);

	if ((rstsr & RCC_C1BOOTRSTSCLRR_PADRSTF) == 0U) {
		if ((rstsr & RCC_C1BOOTRSTSCLRR_STBYC1RSTF) != 0U) {
			INFO("System exits from Standby for CA35\n");
			return;
		}

		if ((rstsr & RCC_C1BOOTRSTSCLRR_D1STBYRSTF) != 0U) {
			INFO("D1 domain exits from DStandby\n");
			return;
		}
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_PORRSTF) != 0U) {
		INFO("  Power-on Reset (rst_por)\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_BORRSTF) != 0U) {
		INFO("  Brownout Reset (rst_bor)\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSSETR_SYSC2RSTF) != 0U) {
		INFO("  System reset (SYSRST) by M33\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSSETR_SYSC1RSTF) != 0U) {
		INFO("  System reset (SYSRST) by A35\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_HCSSRSTF) != 0U) {
		INFO("  Clock failure on HSE\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_IWDG1SYSRSTF) != 0U) {
		INFO("  IWDG1 system reset (rst_iwdg1)\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_IWDG2SYSRSTF) != 0U) {
		INFO("  IWDG2 system reset (rst_iwdg2)\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_IWDG3SYSRSTF) != 0U) {
		INFO("  IWDG3 system reset (rst_iwdg3)\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_IWDG4SYSRSTF) != 0U) {
		INFO("  IWDG4 system reset (rst_iwdg4)\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_IWDG5SYSRSTF) != 0U) {
		INFO("  IWDG5 system reset (rst_iwdg5)\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_C1P1RSTF) != 0U) {
		INFO("  A35 processor core 1 reset\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_PADRSTF) != 0U) {
		INFO("  Pad Reset from NRST\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_VCORERSTF) != 0U) {
		INFO("  Reset due to a failure of VDD_CORE\n");
		return;
	}

	if ((rstsr & RCC_C1BOOTRSTSCLRR_C1RSTF) != 0U) {
		INFO("  A35 processor reset\n");
		return;
	}

	ERROR("  Unidentified reset reason\n");
}

void bl2_el3_early_platform_setup(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
	stm32mp_save_boot_ctx_address(BOOT_CTX_ADDR);
}

void bl2_platform_setup(void)
{
}

static void reset_backup_domain(void)
{
	uintptr_t pwr_base = stm32mp_pwr_base();
	uintptr_t rcc_base = stm32mp_rcc_base();

	/*
	 * Disable the backup domain write protection.
	 * The protection is enable at each reset by hardware
	 * and must be disabled by software.
	 */
	mmio_setbits_32(pwr_base + PWR_BDCR1, PWR_BDCR1_DBD3P);

	while ((mmio_read_32(pwr_base + PWR_BDCR1) & PWR_BDCR1_DBD3P) == 0U) {
		;
	}

	/* Reset backup domain on cold boot cases */
	if ((mmio_read_32(rcc_base + RCC_BDCR) & RCC_BDCR_RTCCKEN) == 0U) {
		mmio_setbits_32(rcc_base + RCC_BDCR, RCC_BDCR_VSWRST);

		while ((mmio_read_32(rcc_base + RCC_BDCR) & RCC_BDCR_VSWRST) == 0U) {
			;
		}

		mmio_clrbits_32(rcc_base + RCC_BDCR, RCC_BDCR_VSWRST);
	}
}

void bl2_el3_plat_arch_setup(void)
{
	const char *board_model;
	boot_api_context_t *boot_context =
		(boot_api_context_t *)stm32mp_get_boot_ctx_address();

	if (stm32_otp_probe() != 0U) {
		EARLY_ERROR("OTP probe failed\n");
		panic();
	}

	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	configure_mmu();

	/* Prevent corruption of preloaded Device Tree */
	mmap_add_dynamic_region(DTB_BASE, DTB_BASE,
				DTB_LIMIT - DTB_BASE,
				MT_RO_DATA | MT_SECURE);

	if (dt_open_and_check(STM32MP_DTB_BASE) < 0) {
		panic();
	}

	reset_backup_domain();

	if (stm32mp2_clk_init() < 0) {
		panic();
	}

	stm32_save_boot_info(boot_context);

	if (stm32mp_uart_console_setup() != 0) {
		goto skip_console_init;
	}

	stm32mp_print_cpuinfo();

	board_model = dt_get_board_model();
	if (board_model != NULL) {
		NOTICE("Model: %s\n", board_model);
	}

	stm32mp_print_boardinfo();

	print_reset_reason();

skip_console_init:
	fconf_populate("TB_FW", STM32MP_DTB_BASE);

	stm32mp_io_setup();
}
