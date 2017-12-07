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

#define CONSOLE_T_NEXT			(U(0) * REGSZ)
#define CONSOLE_T_FLAGS			(U(1) * REGSZ)
#define CONSOLE_T_PUTC			(U(2) * REGSZ)
#define CONSOLE_T_GETC			(U(3) * REGSZ)
#define CONSOLE_T_FLUSH			(U(4) * REGSZ)
#define CONSOLE_T_DRVDATA		(U(5) * REGSZ)

#define CONSOLE_FLAG_BOOT_SHIFT		U(0)
#define CONSOLE_FLAG_RUNTIME_SHIFT	U(1)
#define CONSOLE_FLAG_CRASH_SHIFT	U(2)
#define CONSOLE_FLAG_SCOPE_MASK		U((1 << 8) - 1)

#define ERROR_NO_VALID_CONSOLE		(-128)

#ifndef __ASSEMBLY__

#include <types.h>

typedef struct console {
	struct console *next;
	u_register_t flags;
	void (*putc)(int character, uintptr_t baseaddr);
	int (*getc)(uintptr_t baseaddr);
	int (*flush)(uintptr_t baseaddr);
	/* Additional private driver data may follow here. */
} console_t;

/* Remove a single console_t instance from the console list. */
int console_unregister(console_t *console);
/* Clear the console list (remove all console instances). */
void console_uninit(void);
/* Switch to a new global console state (1 << CONSOLE_FLAG_XXX_SHIFT). */
void console_switch_state(uint32_t new_state);
/* Set scope mask of a console that determines in what states it is active. */
void console_set_scope(console_t *console, uint8_t scope);

int console_putc(int c);
int console_getc(void);
int console_flush(void);

/* DEPRECATED on AArch64 -- use console_<driver>_register() instead! */
int console_init(uintptr_t base_addr,
		 unsigned int uart_clk, unsigned int baud_rate);

#endif /* __ASSEMBLY__ */

#endif /* __CONSOLE_H__ */

