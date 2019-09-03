/*
 * Copyright (c) 2019, Socionext Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/console.h>
#include <errno.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include "uniphier.h"
#include "uniphier_console.h"

#define UNIPHIER_UART_BASE	0x54006800
#define UNIPHIER_UART_END	0x54006c00
#define UNIPHIER_UART_OFFSET	0x100

struct uniphier_console {
	struct console console;
	uintptr_t base;
};

/* These callbacks are implemented in assembly to use crash_console_helpers.S */
int uniphier_console_putc(int character, struct console *console);
int uniphier_console_getc(struct console *console);
int uniphier_console_flush(struct console *console);

static struct uniphier_console uniphier_console = {
	.console = {
		.flags = CONSOLE_FLAG_BOOT |
#if DEBUG
			 CONSOLE_FLAG_RUNTIME |
#endif
			 CONSOLE_FLAG_CRASH |
			 CONSOLE_FLAG_TRANSLATE_CRLF,
		.putc = uniphier_console_putc,
		.getc = uniphier_console_getc,
		.flush = uniphier_console_flush,
	},
};

/*
 * There are 4 UART ports available on this platform. By default, we want to
 * use the same one as used in the previous firmware stage.
 */
static uintptr_t uniphier_console_get_base(void)
{
	uintptr_t base = UNIPHIER_UART_BASE;
	uint32_t div;

	while (base < UNIPHIER_UART_END) {
		div = mmio_read_32(base + UNIPHIER_UART_DLR);
		if (div)
			return base;
		base += UNIPHIER_UART_OFFSET;
	}

	return 0;
}

static void uniphier_console_init(uintptr_t base)
{
	mmio_write_32(base + UNIPHIER_UART_FCR, UNIPHIER_UART_FCR_ENABLE_FIFO);
	mmio_write_32(base + UNIPHIER_UART_LCR_MCR,
		      UNIPHIER_UART_LCR_WLEN8 << 8);
}

void uniphier_console_setup(void)
{
	uintptr_t base;

	base = uniphier_console_get_base();
	if (!base)
		plat_error_handler(-EINVAL);

	uniphier_console.base = base;
	console_register(&uniphier_console.console);

	/*
	 * The hardware might be still printing characters queued up in the
	 * previous firmware stage. Make sure the transmitter is empty before
	 * any initialization. Otherwise, the console might get corrupted.
	 */
	console_flush();

	uniphier_console_init(base);
}
