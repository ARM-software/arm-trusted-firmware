/*
 * Copyright (c) 2014-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stddef.h>

#include <platform_def.h>

#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>

/* Storage for a fixed maximum number of IO entities, definable by platform */
static io_entity_t entity_pool[MAX_IO_HANDLES];

/* Simple way of tracking used storage - each entry is NULL or a pointer to an
 * entity */
static io_entity_t *entity_map[MAX_IO_HANDLES];

/* Track number of allocated entities */
static unsigned int entity_count;

/* Array of fixed maximum of registered devices, definable by platform */
static const io_dev_info_t *devices[MAX_IO_DEVICES];

/* Number of currently registered devices */
static unsigned int dev_count;

/* Extra validation functions only used when asserts are enabled */
#if ENABLE_ASSERTIONS

/* Return a boolean value indicating whether a device connector is valid */
static bool is_valid_dev_connector(const io_dev_connector_t *dev_con)
{
	return (dev_con != NULL) && (dev_con->dev_open != NULL);
}

/* Return a boolean value indicating whether a device handle is valid */
static bool is_valid_dev(const uintptr_t dev_handle)
{
	const io_dev_info_t *dev = (io_dev_info_t *)dev_handle;

	return (dev != NULL) && (dev->funcs != NULL) &&
			(dev->funcs->type != NULL) &&
			(dev->funcs->type() < IO_TYPE_MAX);
}


/* Return a boolean value indicating whether an IO entity is valid */
static bool is_valid_entity(const uintptr_t handle)
{
	const io_entity_t *entity = (io_entity_t *)handle;

	return (entity != NULL) &&
			(is_valid_dev((uintptr_t)entity->dev_handle));
}


/* Return a boolean value indicating whether a seek mode is valid */
static bool is_valid_seek_mode(io_seek_mode_t mode)
{
	return ((mode != IO_SEEK_INVALID) && (mode < IO_SEEK_MAX));
}

#endif /* ENABLE_ASSERTIONS */
/* End of extra validation functions only used when asserts are enabled */


/* Open a connection to a specific device */
static int io_storage_dev_open(const io_dev_connector_t *dev_con,
		const uintptr_t dev_spec,
		io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	assert(is_valid_dev_connector(dev_con));

	return dev_con->dev_open(dev_spec, dev_info);
}


/* Set a handle to track an entity */
static void set_handle(uintptr_t *handle, io_entity_t *entity)
{
	assert(handle != NULL);
	*handle = (uintptr_t)entity;
}


/* Locate an entity in the pool, specified by address */
static int find_first_entity(const io_entity_t *entity, unsigned int *index_out)
{
	int result = -ENOENT;
	for (unsigned int index = 0; index < MAX_IO_HANDLES; ++index) {
		if (entity_map[index] == entity) {
			result = 0;
			*index_out = index;
			break;
		}
	}
	return result;
}


/* Allocate an entity from the pool and return a pointer to it */
static int allocate_entity(io_entity_t **entity)
{
	int result = -ENOMEM;
	assert(entity != NULL);

	if (entity_count < MAX_IO_HANDLES) {
		unsigned int index = 0;
		result = find_first_entity(NULL, &index);
		assert(result == 0);
		*entity = &entity_pool[index];
		entity_map[index] = &entity_pool[index];
		++entity_count;
	}

	return result;
}


/* Release an entity back to the pool */
static int free_entity(const io_entity_t *entity)
{
	int result;
	unsigned int index = 0;
	assert(entity != NULL);

	result = find_first_entity(entity, &index);
	if (result ==  0) {
		entity_map[index] = NULL;
		--entity_count;
	}

	return result;
}


/* Exported API */

/* Register a device driver */
int io_register_device(const io_dev_info_t *dev_info)
{
	int result = -ENOMEM;
	assert(dev_info != NULL);

	if (dev_count < MAX_IO_DEVICES) {
		devices[dev_count] = dev_info;
		dev_count++;
		result = 0;
	}

	return result;
}


/* Open a connection to an IO device */
int io_dev_open(const io_dev_connector_t *dev_con, const uintptr_t dev_spec,
		uintptr_t *handle)
{
	assert(handle != NULL);
	return io_storage_dev_open(dev_con, dev_spec, (io_dev_info_t **)handle);
}


/* Initialise an IO device explicitly - to permit lazy initialisation or
 * re-initialisation */
int io_dev_init(uintptr_t dev_handle, const uintptr_t init_params)
{
	int result = 0;
	assert(dev_handle != (uintptr_t)NULL);
	assert(is_valid_dev(dev_handle));

	io_dev_info_t *dev = (io_dev_info_t *)dev_handle;

	/* Absence of registered function implies NOP here */
	if (dev->funcs->dev_init != NULL) {
		result = dev->funcs->dev_init(dev, init_params);
	}

	return result;
}

/* Close a connection to a device */
int io_dev_close(uintptr_t dev_handle)
{
	int result = 0;
	assert(dev_handle != (uintptr_t)NULL);
	assert(is_valid_dev(dev_handle));

	io_dev_info_t *dev = (io_dev_info_t *)dev_handle;

	/* Absence of registered function implies NOP here */
	if (dev->funcs->dev_close != NULL) {
		result = dev->funcs->dev_close(dev);
	}

	return result;
}


/* Synchronous operations */


/* Open an IO entity */
int io_open(uintptr_t dev_handle, const uintptr_t spec, uintptr_t *handle)
{
	int result;
	assert((spec != (uintptr_t)NULL) && (handle != NULL));
	assert(is_valid_dev(dev_handle));

	io_dev_info_t *dev = (io_dev_info_t *)dev_handle;
	io_entity_t *entity;

	result = allocate_entity(&entity);

	if (result == 0) {
		assert(dev->funcs->open != NULL);
		result = dev->funcs->open(dev, spec, entity);

		if (result == 0) {
			entity->dev_handle = dev;
			set_handle(handle, entity);
		} else
			free_entity(entity);
	}
	return result;
}


/* Seek to a specific position in an IO entity */
int io_seek(uintptr_t handle, io_seek_mode_t mode, signed long long offset)
{
	int result = -ENODEV;
	assert(is_valid_entity(handle) && is_valid_seek_mode(mode));

	io_entity_t *entity = (io_entity_t *)handle;

	io_dev_info_t *dev = entity->dev_handle;

	if (dev->funcs->seek != NULL)
		result = dev->funcs->seek(entity, mode, offset);

	return result;
}


/* Determine the length of an IO entity */
int io_size(uintptr_t handle, size_t *length)
{
	int result = -ENODEV;
	assert(is_valid_entity(handle) && (length != NULL));

	io_entity_t *entity = (io_entity_t *)handle;

	io_dev_info_t *dev = entity->dev_handle;

	if (dev->funcs->size != NULL)
		result = dev->funcs->size(entity, length);

	return result;
}


/* Read data from an IO entity */
int io_read(uintptr_t handle,
		uintptr_t buffer,
		size_t length,
		size_t *length_read)
{
	int result = -ENODEV;
	assert(is_valid_entity(handle));

	io_entity_t *entity = (io_entity_t *)handle;

	io_dev_info_t *dev = entity->dev_handle;

	if (dev->funcs->read != NULL)
		result = dev->funcs->read(entity, buffer, length, length_read);

	return result;
}


/* Write data to an IO entity */
int io_write(uintptr_t handle,
		const uintptr_t buffer,
		size_t length,
		size_t *length_written)
{
	int result = -ENODEV;
	assert(is_valid_entity(handle));

	io_entity_t *entity = (io_entity_t *)handle;

	io_dev_info_t *dev = entity->dev_handle;

	if (dev->funcs->write != NULL) {
		result = dev->funcs->write(entity, buffer, length,
				length_written);
	}

	return result;
}


/* Close an IO entity */
int io_close(uintptr_t handle)
{
	int result = 0;
	assert(is_valid_entity(handle));

	io_entity_t *entity = (io_entity_t *)handle;

	io_dev_info_t *dev = entity->dev_handle;

	/* Absence of registered function implies NOP here */
	if (dev->funcs->close != NULL)
		result = dev->funcs->close(entity);

	/* Ignore improbable free_entity failure */
	(void)free_entity(entity);

	return result;
}
