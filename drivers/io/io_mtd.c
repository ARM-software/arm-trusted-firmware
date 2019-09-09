/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_mtd.h>
#include <lib/utils.h>

typedef struct {
	io_mtd_dev_spec_t	*dev_spec;
	uintptr_t		base;
	unsigned long long	offset;		/* Offset in bytes */
	unsigned long long	size;	/* Size of device in bytes */
} mtd_dev_state_t;

io_type_t device_type_mtd(void);

static int mtd_open(io_dev_info_t *dev_info, const uintptr_t spec,
		    io_entity_t *entity);
static int mtd_seek(io_entity_t *entity, int mode, signed long long offset);
static int mtd_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		    size_t *length_read);
static int mtd_close(io_entity_t *entity);
static int mtd_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int mtd_dev_close(io_dev_info_t *dev_info);

static const io_dev_connector_t mtd_dev_connector = {
	.dev_open	= mtd_dev_open
};

static const io_dev_funcs_t mtd_dev_funcs = {
	.type		= device_type_mtd,
	.open		= mtd_open,
	.seek		= mtd_seek,
	.read		= mtd_read,
	.close		= mtd_close,
	.dev_close	= mtd_dev_close,
};

static mtd_dev_state_t state_pool[MAX_IO_MTD_DEVICES];
static io_dev_info_t dev_info_pool[MAX_IO_MTD_DEVICES];

io_type_t device_type_mtd(void)
{
	return IO_TYPE_MTD;
}

/* Locate a MTD state in the pool, specified by address */
static int find_first_mtd_state(const io_mtd_dev_spec_t *dev_spec,
				unsigned int *index_out)
{
	unsigned int index;
	int result = -ENOENT;

	for (index = 0U; index < MAX_IO_MTD_DEVICES; index++) {
		/* dev_spec is used as identifier since it's unique */
		if (state_pool[index].dev_spec == dev_spec) {
			result = 0;
			*index_out = index;
			break;
		}
	}

	return result;
}

/* Allocate a device info from the pool */
static int allocate_dev_info(io_dev_info_t **dev_info)
{
	unsigned int index = 0U;
	int result;

	result = find_first_mtd_state(NULL, &index);
	if (result != 0) {
		return -ENOMEM;
	}

	dev_info_pool[index].funcs = &mtd_dev_funcs;
	dev_info_pool[index].info = (uintptr_t)&state_pool[index];
	*dev_info = &dev_info_pool[index];

	return 0;
}

/* Release a device info from the pool */
static int free_dev_info(io_dev_info_t *dev_info)
{
	int result;
	unsigned int index = 0U;
	mtd_dev_state_t *state;

	state = (mtd_dev_state_t *)dev_info->info;
	result = find_first_mtd_state(state->dev_spec, &index);
	if (result != 0) {
		return result;
	}

	zeromem(state, sizeof(mtd_dev_state_t));
	zeromem(dev_info, sizeof(io_dev_info_t));

	return 0;
}

static int mtd_open(io_dev_info_t *dev_info, const uintptr_t spec,
		    io_entity_t *entity)
{
	mtd_dev_state_t *cur;

	assert((dev_info->info != 0UL) && (entity->info == 0UL));

	cur = (mtd_dev_state_t *)dev_info->info;
	entity->info = (uintptr_t)cur;
	cur->offset = 0U;

	return 0;
}

/* Seek to a specific position using offset */
static int mtd_seek(io_entity_t *entity, int mode, signed long long offset)
{
	mtd_dev_state_t *cur;

	assert((entity->info != (uintptr_t)NULL) && (offset >= 0));

	cur = (mtd_dev_state_t *)entity->info;

	switch (mode) {
	case IO_SEEK_SET:
		if ((offset >= 0) &&
		    ((unsigned long long)offset >= cur->size)) {
			return -EINVAL;
		}

		cur->offset = offset;
		break;
	case IO_SEEK_CUR:
		if (((cur->offset + (unsigned long long)offset) >=
		     cur->size) ||
		    ((cur->offset + (unsigned long long)offset) <
		     cur->offset)) {
			return -EINVAL;
		}

		cur->offset += (unsigned long long)offset;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int mtd_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		    size_t *out_length)
{
	mtd_dev_state_t *cur;
	io_mtd_ops_t *ops;
	int ret;

	assert(entity->info != (uintptr_t)NULL);
	assert((length > 0U) && (buffer != (uintptr_t)NULL));

	cur = (mtd_dev_state_t *)entity->info;
	ops = &cur->dev_spec->ops;
	assert(ops->read != NULL);

	VERBOSE("Read at %llx into %lx, length %zi\n",
		cur->offset, buffer, length);
	if ((cur->offset + length) > cur->dev_spec->device_size) {
		return -EINVAL;
	}

	ret = ops->read(cur->offset, buffer, length, out_length);
	if (ret < 0) {
		return ret;
	}

	assert(*out_length == length);
	cur->offset += *out_length;

	return 0;
}

static int mtd_close(io_entity_t *entity)
{
	entity->info = (uintptr_t)NULL;

	return 0;
}

static int mtd_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info)
{
	mtd_dev_state_t *cur;
	io_dev_info_t *info;
	io_mtd_ops_t *ops;
	int result;

	result = allocate_dev_info(&info);
	if (result != 0) {
		return -ENOENT;
	}

	cur = (mtd_dev_state_t *)info->info;
	cur->dev_spec = (io_mtd_dev_spec_t *)dev_spec;
	*dev_info = info;
	ops = &(cur->dev_spec->ops);
	if (ops->init != NULL) {
		result = ops->init(&cur->dev_spec->device_size,
				   &cur->dev_spec->erase_size);
	}

	if (result == 0) {
		cur->size = cur->dev_spec->device_size;
	} else {
		cur->size = 0ULL;
	}

	return result;
}

static int mtd_dev_close(io_dev_info_t *dev_info)
{
	return free_dev_info(dev_info);
}

/* Exported functions */

/* Register the MTD driver in the IO abstraction */
int register_io_dev_mtd(const io_dev_connector_t **dev_con)
{
	int result;

	result = io_register_device(&dev_info_pool[0]);
	if (result == 0) {
		*dev_con = &mtd_dev_connector;
	}

	return result;
}
