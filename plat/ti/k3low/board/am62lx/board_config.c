/*
 * Copyright (c) 2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include <k3_console.h>
#include <plat_private.h>
#include <plat_utils.h>

#define PADCONF_ADDR (0x4084000)
#define PAD_MUXMODE_MASK GENMASK(3, 0)
#define PAD_PULLUPDOWN_MASK GENMASK(16, 16)
#define PAD_INPUTACTIVE_MASK GENMASK(18, 18)
#define PAD_DRVDISABLE_MASK GENMASK(21, 21)

#define PIN_MAIN_UART0_RXD (0x01B4)
#define PIN_MAIN_UART0_TXD (0x01B8)
#define PIN_WKUP_UART0_RXD (0x0000)
#define PIN_WKUP_UART0_TXD (0x0004)

void board_init(void)
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

	/* Set WKUP UART pins */
	pad_conf = mmio_read_32(PADCONF_ADDR + PIN_WKUP_UART0_RXD);
	pad_conf &= ~(PAD_MUXMODE_MASK | PAD_DRVDISABLE_MASK);
	pad_conf |= (FIELD_PREP(PAD_PULLUPDOWN_MASK, 1) | FIELD_PREP(PAD_INPUTACTIVE_MASK, 1));
	mmio_write_32(PADCONF_ADDR + PIN_WKUP_UART0_RXD, pad_conf);

	pad_conf = mmio_read_32(PADCONF_ADDR + PIN_WKUP_UART0_TXD);
	pad_conf &= ~(PAD_MUXMODE_MASK | PAD_DRVDISABLE_MASK | PAD_INPUTACTIVE_MASK);
	pad_conf |= FIELD_PREP(PAD_PULLUPDOWN_MASK, 1);
	mmio_write_32(PADCONF_ADDR + PIN_WKUP_UART0_TXD, pad_conf);

	/* Initialize the console to provide early debug support */
	k3_console_setup();
}
