/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UNIPHIER_CONSOLE_H
#define UNIPHIER_CONSOLE_H

#define UNIPHIER_UART_RX	0x00	/* In:  Receive buffer */
#define UNIPHIER_UART_TX	0x00	/* Out: Transmit buffer */

#define UNIPHIER_UART_FCR	0x0c	/* Char/FIFO Control Register */
#define   UNIPHIER_UART_FCR_ENABLE_FIFO	0x01	/* Enable the FIFO */

#define UNIPHIER_UART_LCR_MCR	0x10	/* Line/Modem Control Register */
#define   UNIPHIER_UART_LCR_WLEN8	0x03	/* Wordlength: 8 bits */
#define UNIPHIER_UART_LSR	0x14	/* Line Status Register */
#define   UNIPHIER_UART_LSR_TEMT	0x40	/* Transmitter empty */
#define   UNIPHIER_UART_LSR_TEMT_BIT	6	/* Transmitter empty */
#define   UNIPHIER_UART_LSR_THRE_BIT	5	/* Transmit-hold-register empty */
#define   UNIPHIER_UART_LSR_DR_BIT	0	/* Receiver data ready */
#define UNIPHIER_UART_DLR	0x24	/* Divisor Latch Register */

#endif /* UNIPHIER_CONSOLE_H */
