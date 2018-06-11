/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <desc_image_load.h>
#include <platform_def.h>
#include <xlat_mmu_helpers.h>
#include <xlat_tables_defs.h>
#include <aips.h>
#include <clock.h>
#include <mxc_console.h>
#include "warp7_private.h"

#define UART1_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_UART1_CLK_ROOT_OSC_24M)

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return 0;
}

void bl2_el3_plat_arch_setup(void)
{
	/* Setup the MMU here */
}

/*
 * bl2_early_platform_setup()
 * MMU off
 */
void bl2_el3_early_platform_setup(u_register_t arg1, u_register_t arg2,
				  u_register_t arg3, u_register_t arg4)
{
	uint32_t uart_en_bits = (uint32_t)UART1_CLK_SELECT;

	/* Initialize the AIPS */
	aips_init();

	/* Initialize clocks, regulators, pin-muxes etc */
	clock_init();
	clock_enable_uart(0, uart_en_bits);

	/* Init UART, storage and friends */
	console_init(PLAT_WARP7_BOOT_UART_BASE, PLAT_WARP7_BOOT_UART_CLK_IN_HZ,
		     PLAT_WARP7_CONSOLE_BAUDRATE);
}

/*
 * bl2_platform_setup()
 * MMU on - enabled by bl2_el3_plat_arch_setup()
 */
void bl2_platform_setup(void)
{
}
