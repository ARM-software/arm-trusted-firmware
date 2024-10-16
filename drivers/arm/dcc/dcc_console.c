/*
 * Copyright (c) 2015-2022, Xilinx Inc.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 * Written by Michal Simek.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

#include <errno.h>
#include <stddef.h>
#include <arch_helpers.h>
#include <drivers/arm/dcc.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

/* DCC Status Bits */
#define DCC_STATUS_RX		BIT(30)
#define DCC_STATUS_TX		BIT(29)
#define TIMEOUT_COUNT_US	U(0x10624)

struct dcc_console {
	console_t console;
};

static inline uint32_t __dcc_getstatus(void)
{
	return read_mdccsr_el0();
}

#if ENABLE_CONSOLE_GETC
static inline char __dcc_getchar(void)
{
	char c;

	c = read_dbgdtrrx_el0();

	return c;
}
#endif

static inline void __dcc_putchar(char c)
{
	/*
	 * The typecast is to make absolutely certain that 'c' is
	 * zero-extended.
	 */
	write_dbgdtrtx_el0((unsigned char)c);
}

static int32_t dcc_status_timeout(uint32_t mask)
{
	const unsigned int timeout_count = TIMEOUT_COUNT_US;
	uint64_t timeout;
	unsigned int status;

	timeout = timeout_init_us(timeout_count);

	do {
		status = (__dcc_getstatus() & mask);
		if (timeout_elapsed(timeout)) {
			return -ETIMEDOUT;
		}
	} while ((status != 0U));

	return 0;
}

static int32_t dcc_console_putc(int32_t ch, struct console *console)
{
	unsigned int status;

	status = dcc_status_timeout(DCC_STATUS_TX);
	if (status != 0U) {
		return status;
	}
	__dcc_putchar(ch);

	return ch;
}

#if ENABLE_CONSOLE_GETC
static int32_t dcc_console_getc(struct console *console)
{
	unsigned int status;

	status = dcc_status_timeout(DCC_STATUS_RX);
	if (status != 0U) {
		return status;
	}

	return __dcc_getchar();
}
#endif

/**
 * dcc_console_flush() - Function to force a write of all buffered data
 *		          that hasn't been output.
 * @console		Console struct
 *
 */
static void dcc_console_flush(struct console *console)
{
	unsigned int status;

	status = dcc_status_timeout(DCC_STATUS_TX);
	if (status != 0U) {
		return;
	}
}

static struct dcc_console dcc_console = {
	.console = {
		.flags = CONSOLE_FLAG_BOOT |
			CONSOLE_FLAG_RUNTIME |
			CONSOLE_FLAG_CRASH,
		.putc = dcc_console_putc,
#if ENABLE_CONSOLE_GETC
		.getc = dcc_console_getc,
#endif
		.flush = dcc_console_flush,
	},
};

int console_dcc_register(console_t *console)
{
	memcpy(console, &dcc_console.console, sizeof(console_t));
	return console_register(console);
}

void console_dcc_unregister(console_t *console)
{
	dcc_console_flush(console);
	(void)console_unregister(console);
}
