/*
 * Copyright (c) 2025, Marvell Technology Group Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TRNG_H__
#define __TRNG_H__

#include <stdint.h>

int mv_trng_get_random32(uint32_t *rand, uint8_t num);

#endif /* __TRNG_H__ */
