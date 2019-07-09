/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MESON_CONSOLE_H
#define MESON_CONSOLE_H

#include <drivers/console.h>

#define CONSOLE_T_MESON_BASE	CONSOLE_T_DRVDATA

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct {
	console_t console;
	uintptr_t base;
} console_meson_t;

/*
 * Initialize a new meson console instance and register it with the console
 * framework. The |console| pointer must point to storage that will be valid
 * for the lifetime of the console, such as a global or static local variable.
 * Its contents will be reinitialized from scratch.
 *
 * NOTE: The clock is actually fixed to 24 MHz. The argument is only there in
 * order to make this function future-proof.
 */
int console_meson_register(uintptr_t baseaddr, uint32_t clock, uint32_t baud,
			   console_meson_t *console);

#endif /*__ASSEMBLER__*/

#endif /* MESON_CONSOLE_H */
