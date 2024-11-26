/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <linflex.h>
#include <plat_console.h>
#include <platform_def.h>

void console_s32g2_register(void)
{
	static console_t s32g2_console = {
		.next = NULL,
		.flags = 0u,
	};
	int ret;

	ret = mmap_add_dynamic_region(UART_BASE, UART_BASE, PAGE_SIZE,
				      MT_DEVICE | MT_RW | MT_SECURE);
	if (ret != 0) {
		panic();
	}

	ret = console_linflex_register(UART_BASE, UART_CLOCK_HZ,
				       UART_BAUDRATE, &s32g2_console);
	if (ret == 0) {
		panic();
	}

	console_set_scope(&s32g2_console,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH |
			  CONSOLE_FLAG_TRANSLATE_CRLF);
}
