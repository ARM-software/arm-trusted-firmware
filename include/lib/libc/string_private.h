/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STRING_PRIVATE_H
#define STRING_PRIVATE_H

/* Do not include outside of the libc. Use string.h instead. */

#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t len);
int memcmp(const void *s1, const void *s2, size_t len);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
void *memchr(const void *src, int c, size_t len);
char *strchr(const char *s, int c);
void *memset(void *dst, int val, size_t count);
size_t strlen(const char *s);
char *strrchr(const char *p, int ch);

#endif /* STRING_PRIVATE_H */
