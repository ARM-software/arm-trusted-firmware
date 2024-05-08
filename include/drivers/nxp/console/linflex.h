/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef LINFLEX_H
#define LINFLEX_H

#ifndef __ASSEMBLER__
#include <drivers/console.h>

int console_linflex_core_init(uintptr_t baseaddr, uint32_t clock,
			      uint32_t baud);
int console_linflex_register(uintptr_t baseaddr, uint32_t clock,
			     uint32_t baud, console_t *console);
#endif

#endif /* LINFLEX_H */
