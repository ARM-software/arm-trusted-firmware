/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef UART8250_H
#define UART8250_H

/* UART register */
#define UART_RBR		0x00	/* Receive buffer register */
#define UART_DLL		0x00	/* Divisor latch lsb */
#define UART_THR		0x00	/* Transmit holding register */
#define UART_DLH		0x04	/* Divisor latch msb */
#define UART_IER		0x04	/* Interrupt enable register */
#define UART_FCR		0x08	/* FIFO control register */
#define UART_LCR		0x0c	/* Line control register */
#define UART_MCR		0x10	/* Modem control register */
#define UART_LSR		0x14	/* Line status register */
#define UART_HIGHSPEED		0x24	/* High speed UART */

/* FCR */
#define UART_FCR_FIFO_EN	0x01	/* enable FIFO */
#define UART_FCR_CLEAR_RCVR	0x02	/* clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT	0x04	/* clear the XMIT FIFO */

/* LCR */
#define UART_LCR_WLS_8		0x03	/* 8 bit character length */
#define UART_LCR_DLAB		0x80	/* divisor latch access bit */

/* MCR */
#define UART_MCR_DTR		0x01
#define UART_MCR_RTS		0x02

/* LSR */
#define UART_LSR_DR		0x01	/* Data ready */
#define UART_LSR_THRE		0x20	/* Xmit holding register empty */

#endif /* UART8250_H */
