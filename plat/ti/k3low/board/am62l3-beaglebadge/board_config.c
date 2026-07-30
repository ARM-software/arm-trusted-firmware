/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/cadence/cdns_nand.h>
#include <lib/mmio.h>

#include <am62l_board_config.h>
#include <k3_console.h>
#include <plat_private.h>
#include <platform_def.h>

static void uart_padconfig_setup(void)
{
	uint32_t pad_conf;

	/* Set main UART0 pins */
	pad_conf = mmio_read_32(PADCONF_ADDR + PIN_MAIN_UART0_RXD);
	pad_conf &= ~(PAD_MUXMODE_MASK | PAD_DRVDISABLE_MASK);
	pad_conf |= (FIELD_PREP(PAD_PULLUPDOWN_MASK, 1) | FIELD_PREP(PAD_INPUTACTIVE_MASK, 1));
	mmio_write_32(PADCONF_ADDR + PIN_MAIN_UART0_RXD, pad_conf);

	pad_conf = mmio_read_32(PADCONF_ADDR + PIN_MAIN_UART0_TXD);
	pad_conf &= ~(PAD_MUXMODE_MASK | PAD_DRVDISABLE_MASK | PAD_INPUTACTIVE_MASK);
	pad_conf |= FIELD_PREP(PAD_PULLUPDOWN_MASK, 1);
	mmio_write_32(PADCONF_ADDR + PIN_MAIN_UART0_TXD, pad_conf);
}

void board_init(void)
{
	uart_padconfig_setup();

	/* Initialize the console to provide early debug support */
	k3_console_setup();
}
