/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LS_16550_H
#define LS_16550_H

#include <drivers/console.h>

/* UART16550 Registers */
#define UARTTX			0x0
#define UARTRX			0x0
#define UARTDLL			0x0
#define UARTIER			0x1
#define UARTDLLM		0x1
#define UARTFCR			0x2
#define UARTLCR			0x3
#define UARTLSR			0x5
#define UARTMCR                 0x4

/* FIFO Control Register bits */
#define UARTFCR_FIFOMD_16450	(0 << 6)
#define UARTFCR_FIFOMD_16550	(1 << 6)
#define UARTFCR_RXTRIG_1	(0 << 6)
#define UARTFCR_RXTRIG_4	(1 << 6)
#define UARTFCR_RXTRIG_8	(2 << 6)
#define UARTFCR_RXTRIG_16	(3 << 6)
#define UARTFCR_TXTRIG_1	(0 << 4)
#define UARTFCR_TXTRIG_4	(1 << 4)
#define UARTFCR_TXTRIG_8	(2 << 4)
#define UARTFCR_TXTRIG_16	(3 << 4)
#define UARTFCR_DMAEN		(1 << 3)	/* Enable DMA mode */
#define UARTFCR_TXCLR		(1 << 2)	/* Clear contents of Tx FIFO */
#define UARTFCR_RXCLR		(1 << 1)	/* Clear contents of Rx FIFO */
#define UARTFCR_FIFOEN		(1 << 0)	/* Enable the Tx/Rx FIFO */
#define UARTFCR_64FIFO          (1 << 5)

/* Line Control Register bits */
#define UARTLCR_DLAB		(1 << 7)	/* Divisor Latch Access */
#define UARTLCR_SETB		(1 << 6)	/* Set BREAK Condition */
#define UARTLCR_SETP		(1 << 5)	/* Set Parity to LCR[4] */
#define UARTLCR_EVEN		(1 << 4)	/* Even Parity Format */
#define UARTLCR_PAR		(1 << 3)	/* Parity */
#define UARTLCR_STOP		(1 << 2)	/* Stop Bit */
#define UARTLCR_WORDSZ_5	0		/* Word Length of 5 */
#define UARTLCR_WORDSZ_6	1		/* Word Length of 6 */
#define UARTLCR_WORDSZ_7	2		/* Word Length of 7 */
#define UARTLCR_WORDSZ_8	3		/* Word Length of 8 */

/* Line Status Register bits */
#define UARTLSR_RXFIFOEMT	(1 << 9)	/* Rx Fifo Empty */
#define UARTLSR_TXFIFOFULL	(1 << 8)	/* Tx Fifo Full */
#define UARTLSR_RXFIFOERR	(1 << 7)	/* Rx Fifo Error */
#define UARTLSR_TEMT		(1 << 6)	/* Tx Shift Register Empty */
#define UARTLSR_THRE		(1 << 5)	/* Tx Holding Register Empty */
#define UARTLSR_BRK		(1 << 4)	/* Break Condition Detected */
#define UARTLSR_FERR		(1 << 3)	/* Framing Error */
#define UARTLSR_PERR		(1 << 3)	/* Parity Error */
#define UARTLSR_OVRF		(1 << 2)	/* Rx Overrun Error */
#define UARTLSR_RDR		(1 << 2)	/* Rx Data Ready */

#ifndef __ASSEMBLER__

#include <stdint.h>

/*
 * Initialize a new 16550 console instance and register it with the console
 * framework. The |console| pointer must point to storage that will be valid
 * for the lifetime of the console, such as a global or static local variable.
 * Its contents will be reinitialized from scratch.
 */
int console_ls_16550_register(uintptr_t baseaddr, uint32_t clock, uint32_t baud,
			      console_t *console);

#endif /*__ASSEMBLER__*/

#endif /* LS_16550_H */
