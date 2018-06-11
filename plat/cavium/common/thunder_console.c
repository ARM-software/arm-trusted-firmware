/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <console.h>
#include <pl011.h>
#include <platform_def.h>

#include <thunder_private.h>

#if MULTI_CONSOLE_API
static console_pl011_t console;
#endif /* MULTI_CONSOLE_API */

void cavium_console_init(void)
{
	static const uint8_t div[] = {1, 2, 4, 6, 8, 16, 24, 32};
	cavm_uaax_uctl_ctl_t uaax_uctl_ctl;
	cavm_rst_boot_t rst_boot;
	uint64_t sclk, hclk;

	rst_boot.u = CSR_READ_PA(0, CAVM_RST_BOOT);
	sclk = rst_boot.s.pnr_mul * 50000000UL; //Hz

	uaax_uctl_ctl.u = CSR_READ_PA(0, CAVM_UAAX_UCTL_CTL(0));

	hclk = sclk / div[uaax_uctl_ctl.s.h_clkdiv_sel];

#if MULTI_CONSOLE_API
	(void)console_pl011_register(CSR_PA(0, CAVM_UAAX_PF_BAR0(0)),
				     hclk, PLAT_CAVIUM_CONSOLE_BAUDRATE,
				     &console);
#ifndef DEBUG
	console_set_scope(&console.console, CONSOLE_FLAG_BOOT);
#else
	console_set_scope(&console.console, CONSOLE_FLAG_BOOT |
			  CONSOLE_FLAG_RUNTIME);
#endif
#else
	console_init(CSR_PA(0, CAVM_UAAX_PF_BAR0(0)),
		     hclk,
		     PLAT_CAVIUM_CONSOLE_BAUDRATE);
#endif /* MULTI_CONSOLE_API */
}

void cavium_console_unregister(void)
{
#if !MULTI_CONSOLE_API
	console_uninit();
#endif
}

