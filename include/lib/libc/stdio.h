/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef STDIO_H
#define STDIO_H

#include <stdio_.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define EOF            -1

int printf(const char *fmt, ...);
int snprintf(char *s, size_t n, const char *fmt, ...);
int sprintf(char *s, const char *fmt, ...);
int sscanf(const char *s, const char *fmt, ...);

#ifdef STDARG_H
int vsnprintf(char *s, size_t n, const char *fmt, va_list arg);
int vsprintf(char *s, const char *fmt, va_list arg);
#endif

int putchar(int c);
int puts(const char *s);

#endif /* STDIO_H */
