/*
 * Copyright (c) 2013, ARM Limited and Contributors. All rights reserved.
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

#include <console.h>
#include <platform.h>
#include <pl011.h>

/*
 * TODO: Console init functions shoule be in a console.c. This file should
 * only contain the pl011 accessors.
 */
void console_init(void)
{
	unsigned int divisor;

	/* Baud Rate */

#if defined(PL011_INTEGER) && defined(PL011_FRACTIONAL)
	mmio_write_32(PL011_BASE + UARTIBRD, PL011_INTEGER);
	mmio_write_32(PL011_BASE + UARTFBRD, PL011_FRACTIONAL);
#else
	divisor = (PL011_CLK_IN_HZ * 4) / PL011_BAUDRATE;
	mmio_write_32(PL011_BASE + UARTIBRD, divisor >> 6);
	mmio_write_32(PL011_BASE + UARTFBRD, divisor & 0x3F);
#endif


	mmio_write_32(PL011_BASE + UARTLCR_H, PL011_LINE_CONTROL);

	/* Clear any pending errors */
	mmio_write_32(PL011_BASE + UARTECR, 0);

	/* Enable tx, rx, and uart overall */
	mmio_write_32(PL011_BASE + UARTCR,
		      PL011_UARTCR_RXE | PL011_UARTCR_TXE |
		      PL011_UARTCR_UARTEN);
}

int console_putc(int c)
{
	if (c == '\n') {
		console_putc('\r');
	}
	while ((mmio_read_32(PL011_BASE + UARTFR) & PL011_UARTFR_TXFE)
	       == 0) ;
	mmio_write_32(PL011_BASE + UARTDR, c);
	return c;
}

int console_getc(void)
{
	while ((mmio_read_32(PL011_BASE + UARTFR) & PL011_UARTFR_RXFE)
	       != 0) ;
	return mmio_read_32(PL011_BASE + UARTDR);
}
