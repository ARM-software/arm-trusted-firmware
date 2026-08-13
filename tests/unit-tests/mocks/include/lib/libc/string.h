/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOCK_STRING_H
#define MOCK_STRING_H

#ifdef __cplusplus

extern "C" {
#endif

#include "string_private.h"

void *memmove(void *dst, const void *src, size_t len);
size_t strnlen(const char *s, size_t maxlen);
size_t strlcpy(char *dst, const char *src, size_t dsize);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_STRING_H */
