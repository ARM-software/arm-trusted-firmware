/*
 * Copyright (c) 2012-2017 Roberto E. Vargas Caballero
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Portions copyright (c) 2018-2025, Arm Limited and Contributors.
 * Portions copyright (c) 2023, Intel Corporation. All rights reserved.
 * All rights reserved.
 */

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

/*
 * When conditions are right, the compiler may have a baked-in call that can be
 * inlined and that will be much more optimal than our generic implementation.
 * When it doesn't, it will emit a call to the original function for which we
 * provide an implementation.
 */
#define memcpy  __builtin_memcpy
#define memset  __builtin_memset
#define memcmp  __builtin_memcmp
#define memchr  __builtin_memchr
#define strcmp  __builtin_strcmp
#define strncmp __builtin_strncmp
#define strchr  __builtin_strchr
#define strlen  __builtin_strlen
#define strrchr __builtin_strrchr

int memcpy_s(void *dst, size_t dsize, void *src, size_t ssize);
void *memmove(void *dst, const void *src, size_t len);
void *memrchr(const void *src, int c, size_t len);
size_t strnlen(const char *s, size_t maxlen);
size_t strlcpy(char * dst, const char * src, size_t dsize);
size_t strlcat(char * dst, const char * src, size_t dsize);
char *strtok_r(char *s, const char *delim, char **last);
size_t strnlen_secure(const char *str, size_t maxlen);
int strcpy_secure(char *restrict dest, size_t dest_size, const char *restrict src);

#endif /* STRING_H */
