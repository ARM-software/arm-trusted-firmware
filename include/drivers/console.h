/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <lib/utils_def.h>

#define CONSOLE_T_NEXT			(U(0) * REGSZ)
#define CONSOLE_T_FLAGS			(U(1) * REGSZ)
#define CONSOLE_T_PUTC			(U(2) * REGSZ)
#define CONSOLE_T_GETC			(U(3) * REGSZ)
#define CONSOLE_T_FLUSH			(U(4) * REGSZ)
#define CONSOLE_T_BASE			(U(5) * REGSZ)
#define CONSOLE_T_DRVDATA		(U(6) * REGSZ)

#define CONSOLE_FLAG_BOOT		(U(1) << 0)
#define CONSOLE_FLAG_RUNTIME		(U(1) << 1)
#define CONSOLE_FLAG_CRASH		(U(1) << 2)
/* Bits 3 to 7 reserved for additional scopes in future expansion. */
#define CONSOLE_FLAG_SCOPE_MASK		((U(1) << 8) - 1)
/* Bits 8 to 31 for non-scope use. */
#define CONSOLE_FLAG_TRANSLATE_CRLF	(U(1) << 8)

/* Returned by getc callbacks when receive FIFO is empty. */
#define ERROR_NO_PENDING_CHAR		(-1)
/* Returned by console_xxx() if no registered console implements xxx. */
#define ERROR_NO_VALID_CONSOLE		(-128)

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct console {
	struct console *next;
	/*
	 * Only the low 32 bits are used. The type is u_register_t to align the
	 * fields of the struct to 64 bits in AArch64 and 32 bits in AArch32
	 */
	u_register_t flags;
	int (*const putc)(int character, struct console *console);
	int (*const getc)(struct console *console);
	void (*const flush)(struct console *console);
	uintptr_t base;
	/* Additional private driver data may follow here. */
} console_t;

/* offset macro assertions for console_t */
#include <drivers/console_assertions.h>

/*
 * Add a console_t instance to the console list. This should only be called by
 * console drivers after they have initialized all fields in the console
 * structure. Platforms seeking to register a new console need to call the
 * respective console__register() function instead.
 */
int console_register(console_t *console);
/* Remove a single console_t instance from the console list. Return a pointer to
 * the console that was removed if it was found, or NULL if not. */
console_t *console_unregister(console_t *console);
/* Returns 1 if this console is already registered, 0 if not */
int console_is_registered(console_t *console);
/*
 * Set scope mask of a console that determines in what states it is active.
 * By default they are registered with (CONSOLE_FLAG_BOOT|CONSOLE_FLAG_CRASH).
 */
void console_set_scope(console_t *console, unsigned int scope);

/* Switch to a new global console state (CONSOLE_FLAG_BOOT/RUNTIME/CRASH). */
void console_switch_state(unsigned int new_state);
/* Output a character on all consoles registered for the current state. */
int console_putc(int c);
/* Read a character (blocking) from any console registered for current state. */
int console_getc(void);
/* Flush all consoles registered for the current state. */
void console_flush(void);

#endif /* __ASSEMBLER__ */

#endif /* CONSOLE_H */
