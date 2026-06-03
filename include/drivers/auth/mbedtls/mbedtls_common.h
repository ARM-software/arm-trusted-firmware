/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MBEDTLS_COMMON_H
#define MBEDTLS_COMMON_H

#include <stddef.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <lib/utils.h>

void mbedtls_init(void);

static inline void mbedtls_zero_and_clean(void *ptr, size_t len)
{
	/* Defensive scrub: also push the zeroed state out of D-cache. */
	zeromem(ptr, len);
	clean_dcache_range((uintptr_t)ptr, len);
}

#endif /* MBEDTLS_COMMON_H */
