/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PL011_H
#define PL011_H

#include <drivers/console.h>

/* PL011 Registers */
#define UARTDR                    0x000
#define UARTRSR                   0x004
#define UARTECR                   0x004
#define UARTFR                    0x018
#define UARTIMSC                  0x038
#define UARTRIS                   0x03C
#define UARTICR                   0x044

/* PL011 registers (out of the SBSA specification) */
#if !PL011_GENERIC_UART
#define UARTILPR                  0x020
#define UARTIBRD                  0x024
#define UARTFBRD                  0x028
#define UARTLCR_H                 0x02C
#define UARTCR                    0x030
#define UARTIFLS                  0x034
#define UARTMIS                   0x040
#define UARTDMACR                 0x048
#endif /* !PL011_GENERIC_UART */

/* Data status bits */
#define UART_DATA_ERROR_MASK      0x0F00

/* Status reg bits */
#define UART_STATUS_ERROR_MASK    0x0F

/* Flag reg bits */
#define PL011_UARTFR_RI           (1 << 8)	/* Ring indicator */
#define PL011_UARTFR_TXFE         (1 << 7)	/* Transmit FIFO empty */
#define PL011_UARTFR_RXFF         (1 << 6)	/* Receive  FIFO full */
#define PL011_UARTFR_TXFF         (1 << 5)	/* Transmit FIFO full */
#define PL011_UARTFR_RXFE         (1 << 4)	/* Receive  FIFO empty */
#define PL011_UARTFR_BUSY         (1 << 3)	/* UART busy */
#define PL011_UARTFR_DCD          (1 << 2)	/* Data carrier detect */
#define PL011_UARTFR_DSR          (1 << 1)	/* Data set ready */
#define PL011_UARTFR_CTS          (1 << 0)	/* Clear to send */

#define PL011_UARTFR_TXFF_BIT	5	/* Transmit FIFO full bit in UARTFR register */
#define PL011_UARTFR_RXFE_BIT	4	/* Receive FIFO empty bit in UARTFR register */
#define PL011_UARTFR_BUSY_BIT	3	/* UART busy bit in UARTFR register */

/* Control reg bits */
#if !PL011_GENERIC_UART
#define PL011_UARTCR_CTSEN        (1 << 15)	/* CTS hardware flow control enable */
#define PL011_UARTCR_RTSEN        (1 << 14)	/* RTS hardware flow control enable */
#define PL011_UARTCR_RTS          (1 << 11)	/* Request to send */
#define PL011_UARTCR_DTR          (1 << 10)	/* Data transmit ready. */
#define PL011_UARTCR_RXE          (1 << 9)	/* Receive enable */
#define PL011_UARTCR_TXE          (1 << 8)	/* Transmit enable */
#define PL011_UARTCR_LBE          (1 << 7)	/* Loopback enable */
#define PL011_UARTCR_UARTEN       (1 << 0)	/* UART Enable */

#if !defined(PL011_LINE_CONTROL)
/* FIFO Enabled / No Parity / 8 Data bit / One Stop Bit */
#define PL011_LINE_CONTROL  (PL011_UARTLCR_H_FEN | PL011_UARTLCR_H_WLEN_8)
#endif

/* Line Control Register Bits */
#define PL011_UARTLCR_H_SPS       (1 << 7)	/* Stick parity select */
#define PL011_UARTLCR_H_WLEN_8    (3 << 5)
#define PL011_UARTLCR_H_WLEN_7    (2 << 5)
#define PL011_UARTLCR_H_WLEN_6    (1 << 5)
#define PL011_UARTLCR_H_WLEN_5    (0 << 5)
#define PL011_UARTLCR_H_FEN       (1 << 4)	/* FIFOs Enable */
#define PL011_UARTLCR_H_STP2      (1 << 3)	/* Two stop bits select */
#define PL011_UARTLCR_H_EPS       (1 << 2)	/* Even parity select */
#define PL011_UARTLCR_H_PEN       (1 << 1)	/* Parity Enable */
#define PL011_UARTLCR_H_BRK       (1 << 0)	/* Send break */

#endif /* !PL011_GENERIC_UART */

#define CONSOLE_T_PL011_BASE	CONSOLE_T_DRVDATA

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct {
	console_t console;
	uintptr_t base;
} console_pl011_t;

/*
 * Initialize a new PL011 console instance and register it with the console
 * framework. The |console| pointer must point to storage that will be valid
 * for the lifetime of the console, such as a global or static local variable.
 * Its contents will be reinitialized from scratch.
 */
int console_pl011_register(uintptr_t baseaddr, uint32_t clock, uint32_t baud,
			   console_pl011_t *console);

#endif /*__ASSEMBLER__*/

#endif /* PL011_H */
