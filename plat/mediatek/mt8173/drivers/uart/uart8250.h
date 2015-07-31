/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __UART8250_H__
#define __UART8250_H__

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

#endif	/* __UART8250_H__ */
