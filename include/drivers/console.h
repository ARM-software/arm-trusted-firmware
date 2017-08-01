/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <utils_def.h>

#ifdef AARCH32
#define REGSZ U(4)
#else
#define REGSZ U(8)
#endif

#define CONSOLE_INFO_NEXT	(U(0) * REGSZ)
#define CONSOLE_INFO_FLAGS	(U(1) * REGSZ)
#define CONSOLE_INFO_BASE	(U(2) * REGSZ)
#define CONSOLE_INFO_PUTC	(U(3) * REGSZ)
#define CONSOLE_INFO_GETC	(U(4) * REGSZ)
#define CONSOLE_INFO_FLUSH	(U(5) * REGSZ)

#define CONSOLE_FLAG_BOOT_SHIFT			U(0)
#define CONSOLE_FLAG_RUNTIME_SHIFT		U(1)
#define CONSOLE_FLAG_CRASH_SHIFT		U(2)

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <types.h>

typedef struct console_info {
	struct console_info *next;
	u_register_t flags;
	uintptr_t baseaddr;
	void (*putc)(int character, uintptr_t baseaddr);
	int (*getc)(uintptr_t baseaddr);
	int (*flush)(uintptr_t baseaddr);
} console_info_t;

void console_register(console_info_t *console);
console_info_t *console_unregister(console_info_t *console);
void console_uninit(void);
int console_putc(int c);
int console_getc(void);
int console_flush(void);

/* DEPRECATED on AArch64 -- use console_<driver>_register() instead! */
int console_init(uintptr_t base_addr,
		unsigned int uart_clk, unsigned int baud_rate);

CASSERT(CONSOLE_INFO_NEXT == __builtin_offsetof(console_info_t, next),
	assert_console_info_next_offset_mismatch);
CASSERT(CONSOLE_INFO_FLAGS == __builtin_offsetof(console_info_t, flags),
	assert_console_info_flags_offset_mismatch);
CASSERT(CONSOLE_INFO_BASE == __builtin_offsetof(console_info_t, baseaddr),
	assert_console_info_base_offset_mismatch);
CASSERT(CONSOLE_INFO_PUTC == __builtin_offsetof(console_info_t, putc),
	assert_console_info_putc_offset_mismatch);
CASSERT(CONSOLE_INFO_GETC == __builtin_offsetof(console_info_t, getc),
	assert_console_info_getc_offset_mismatch);
CASSERT(CONSOLE_INFO_FLUSH == __builtin_offsetof(console_info_t, flush),
	assert_console_info_flush_offset_mismatch);

#endif /* __ASSEMBLY__ */

#endif /* __CONSOLE_H__ */

