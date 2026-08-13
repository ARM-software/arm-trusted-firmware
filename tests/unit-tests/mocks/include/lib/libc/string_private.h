/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOCK_STRING_PRIVATE_H
#define MOCK_STRING_PRIVATE_H

#ifdef __cplusplus

extern "C" {
#endif

#include <stddef.h>

typedef unsigned char uint8_t;

void *memcpy(void *dst, const void *src, size_t len);
int memcmp(const void *s1, const void *s2, size_t len);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
void *memchr(const void *src, int c, size_t len);
char *strchr(const char *s, int c);
void *memset(void *dst, int val, size_t count);
size_t strlen(const char *s);
char *strrchr(const char *p, int ch);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_STRING_PRIVATE_H */