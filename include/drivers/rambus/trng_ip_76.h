/*
 * Copyright (c) 2020, Marvell Technology Group Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef __TRNG_IP_76_H__
#define __TRNG_IP_76_H__

#include <stdbool.h>
#include <stdint.h>

int32_t eip76_rng_read_rand_buf(void *data, bool wait);
int32_t eip76_rng_probe(uintptr_t base_addr);
int32_t eip76_rng_get_random(uint8_t *data, uint32_t len);

#endif /* __TRNG_IP_76_H__ */
