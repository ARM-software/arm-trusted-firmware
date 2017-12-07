/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CADENCE_UART_H__
#define __CADENCE_UART_H__

#include <console.h>

/* This is very minimalistic and will only work in QEMU.  */

/* CADENCE Registers */
#define R_UART_CR	0
#define R_UART_CR_RXRST	(1 << 0) /* RX logic reset */
#define R_UART_CR_TXRST	(1 << 1) /* TX logic reset */
#define R_UART_CR_RX_EN	(1 << 2) /* RX enabled */
#define R_UART_CR_TX_EN	(1 << 4) /* TX enabled */

#define R_UART_SR		0x2C
#define UART_SR_INTR_REMPTY_BIT	1
#define UART_SR_INTR_TFUL_BIT	4

#define R_UART_TX	0x30
#define R_UART_RX	0x30

#define CONSOLE_T_CDNS_BASE	CONSOLE_T_DRVDATA

#ifndef __ASSEMBLY__

#include <types.h>

typedef struct {
	console_t console;
	uintptr_t base;
} console_cdns_t;

int console_cdns_register(console_cdns_t *console, uint64_t baseaddr,
			  uint32_t clock, uint32_t baud);

#endif /*__ASSEMBLY__*/

#endif
