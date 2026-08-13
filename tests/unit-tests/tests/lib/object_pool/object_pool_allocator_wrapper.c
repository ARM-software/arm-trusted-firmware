/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "object_pool_allocator_wrapper.h"

#include "lib/object_pool.h"

uint32_t array[POOL_SIZE];

void object_pool_allocate(struct object_pool *pool)
{
	OBJECT_POOL_ARRAY(local_pool, array);
	memcpy(pool, &local_pool, sizeof(*pool));
}
