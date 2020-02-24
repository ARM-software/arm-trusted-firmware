/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OBJECT_POOL_H
#define OBJECT_POOL_H

#include <stdlib.h>

#include <common/debug.h>
#include <lib/utils_def.h>

/*
 * Pool of statically allocated objects.
 *
 * Objects can be reserved but not freed. This is by design and it is not a
 * limitation. We do not want to introduce complexity induced by memory freeing,
 * such as use-after-free bugs, memory fragmentation and so on.
 *
 * The object size and capacity of the pool are fixed at build time. So is the
 * address of the objects back store.
 */
struct object_pool {
	/* Size of 1 object in the pool in byte unit. */
	const size_t obj_size;

	/* Number of objects in the pool. */
	const size_t capacity;

	/* Objects back store. */
	void *const objects;

	/* How many objects are currently allocated. */
	size_t used;
};

/* Create a static pool of objects. */
#define OBJECT_POOL(_pool_name, _obj_backstore, _obj_size, _obj_count)	\
	struct object_pool _pool_name = {				\
		.objects = (_obj_backstore),				\
		.obj_size = (_obj_size),				\
		.capacity = (_obj_count),				\
		.used = 0U,						\
	}

/* Create a static pool of objects out of an array of pre-allocated objects. */
#define OBJECT_POOL_ARRAY(_pool_name, _obj_array)			\
	OBJECT_POOL(_pool_name, (_obj_array),				\
		    sizeof((_obj_array)[0]), ARRAY_SIZE(_obj_array))

/*
 * Allocate 'count' objects from a pool.
 * Return the address of the first object. Panic on error.
 */
static inline void *pool_alloc_n(struct object_pool *pool, size_t count)
{
	if ((pool->used + count) > pool->capacity) {
		ERROR("Cannot allocate %zu objects out of pool (%zu objects left).\n",
		      count, pool->capacity - pool->used);
		panic();
	}

	void *obj = (char *)(pool->objects) + (pool->obj_size * pool->used);
	pool->used += count;
	return obj;
}

/*
 * Allocate 1 object from a pool.
 * Return the address of the object. Panic on error.
 */
static inline void *pool_alloc(struct object_pool *pool)
{
	return pool_alloc_n(pool, 1U);
}

#endif /* OBJECT_POOL_H */
