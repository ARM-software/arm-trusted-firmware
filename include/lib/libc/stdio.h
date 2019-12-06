/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018-2019, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef STDIO_H
#define STDIO_H

#include <cdefs.h>
#include <stddef.h>
#include <stdio_.h>

#define EOF            -1

int printf(const char *fmt, ...) __printflike(1, 2);
int snprintf(char *s, size_t n, const char *fmt, ...) __printflike(3, 4);

#ifdef STDARG_H
int vprintf(const char *fmt, va_list args);
#endif

int putchar(int c);
int puts(const char *s);

#endif /* STDIO_H */
