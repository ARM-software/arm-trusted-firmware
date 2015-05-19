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

#ifndef __UART_16550_H__
#define __UART_16550_H__

/* UART16550 Registers */
#define UARTTX			0x0
#define UARTRX			0x0
#define UARTDLL			0x0
#define UARTIER			0x4
#define UARTDLLM		0x4
#define UARTIIR			0x8
#define UARTFCR			0x8
#define UARTLCR			0xc
#define UARTMCR			0x10
#define UARTLSR			0x14
#define UARTMSR			0x18
#define UARTSPR			0x1c
#define UARTCSR			0x20
#define UARTRXFIFOCFG		0x24
#define UARTMIE			0x28
#define UARTVNDR		0x2c
#define UARTASR			0x3c

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

#endif	/* __UART_16550_H__ */
