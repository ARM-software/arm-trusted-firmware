/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AM62L_BOARD_CONFIG_H
#define AM62L_BOARD_CONFIG_H

#define PADCONF_ADDR		0x4084000
#define PAD_MUXMODE_MASK 	GENMASK(3, 0)
#define PAD_PULLUPDOWN_MASK 	GENMASK(16, 16)
#define PAD_INPUTACTIVE_MASK 	GENMASK(18, 18)
#define PAD_DRVDISABLE_MASK 	GENMASK(21, 21)

#define PIN_MAIN_UART0_RXD 	0x01B4
#define PIN_MAIN_UART0_TXD 	0x01B8
#define PIN_WKUP_UART0_RXD 	0x0000
#define PIN_WKUP_UART0_TXD 	0x0004

#endif /* AM62L_BOARD_CONFIG_H */
