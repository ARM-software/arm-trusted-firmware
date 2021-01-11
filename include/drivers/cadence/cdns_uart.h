/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CDNS_UART_H
#define CDNS_UART_H

#include <drivers/console.h>

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
#define UART_SR_INTR_TEMPTY_BIT	3

#define R_UART_TX	0x30
#define R_UART_RX	0x30

#ifndef __ASSEMBLER__

#include <stdint.h>

/*
 * Initialize a new Cadence console instance and register it with the console
 * framework. The |console| pointer must point to storage that will be valid
 * for the lifetime of the console, such as a global or static local variable.
 * Its contents will be reinitialized from scratch.
 */
int console_cdns_register(uintptr_t baseaddr, uint32_t clock, uint32_t baud,
			  console_t *console);

#endif /*__ASSEMBLER__*/

#endif /* CDNS_UART_H */
