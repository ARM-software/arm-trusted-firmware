/*
 * Copyright (c) 2012-2021 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018-2019, ARM Limited and Contributors.
 * All rights reserved.
 */

#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define _ATEXIT_MAX 1

#define isspace(x)    (((x) == ' ') || ((x) == '\r') || ((x) == '\n') || \
			((x) == '\t') || ((x) == '\b'))

extern void abort(void);
extern int atexit(void (*func)(void));
extern void exit(int status);

long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
#endif /* STDLIB_H */
