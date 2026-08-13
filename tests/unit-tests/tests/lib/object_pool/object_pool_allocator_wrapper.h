/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "lib/object_pool.h"

#define POOL_SIZE (16)
extern uint32_t array[POOL_SIZE];
void object_pool_allocate(struct object_pool *pool);

#ifdef __cplusplus
}
#endif
