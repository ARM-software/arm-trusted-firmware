/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRNG_ENTROPY_POOL_H
#define TRNG_ENTROPY_POOL_H

#include <stdbool.h>
#include <stdint.h>

bool trng_pack_entropy(uint32_t nbits, uint64_t *out);
void trng_entropy_pool_setup(void);

#endif /* TRNG_ENTROPY_POOL_H */
