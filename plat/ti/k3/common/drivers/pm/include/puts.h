/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PUTS_H
#define PUTS_H

#include <types.h>

/*
 * Some output types operate more efficiently when they can queue characters
 * rather than operating on a character at a time. To make this work
 * efficiently with out printf implementation, we need some method to
 * notify the character driver that it needs to flush current output
 */
#define PUTCHAR_FLUSH	-1

int32_t set_putchar(int (*pc)(int));
int32_t putchar(int32_t c);
int32_t puts(const char *str);
int32_t putsn(const char *s, size_t maxlen);

#endif
