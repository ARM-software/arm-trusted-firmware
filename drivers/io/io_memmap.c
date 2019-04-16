/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_storage.h>
#include <lib/utils.h>

/* As we need to be able to keep state for seek, only one file can be open
 * at a time. Make this a structure and point to the entity->info. When we
 * can malloc memory we can change this to support more open files.
 */
typedef struct {
	/* Use the 'in_use' flag as any value for base and file_pos could be
	 * valid.
	 */
	int			in_use;
	uintptr_t		base;
	unsigned long long	file_pos;
	unsigned long long	size;
} file_state_t;

static file_state_t current_file = {0};

/* Identify the device type as memmap */
static io_type_t device_type_memmap(void)
{
	return IO_TYPE_MEMMAP;
}

/* Memmap device functions */
static int memmap_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int memmap_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
			     io_entity_t *entity);
static int memmap_block_seek(io_entity_t *entity, int mode,
			     signed long long offset);
static int memmap_block_len(io_entity_t *entity, size_t *length);
static int memmap_block_read(io_entity_t *entity, uintptr_t buffer,
			     size_t length, size_t *length_read);
static int memmap_block_write(io_entity_t *entity, const uintptr_t buffer,
			      size_t length, size_t *length_written);
static int memmap_block_close(io_entity_t *entity);
static int memmap_dev_close(io_dev_info_t *dev_info);


static const io_dev_connector_t memmap_dev_connector = {
	.dev_open = memmap_dev_open
};


static const io_dev_funcs_t memmap_dev_funcs = {
	.type = device_type_memmap,
	.open = memmap_block_open,
	.seek = memmap_block_seek,
	.size = memmap_block_len,
	.read = memmap_block_read,
	.write = memmap_block_write,
	.close = memmap_block_close,
	.dev_init = NULL,
	.dev_close = memmap_dev_close,
};


/* No state associated with this device so structure can be const */
static const io_dev_info_t memmap_dev_info = {
	.funcs = &memmap_dev_funcs,
	.info = (uintptr_t)NULL
};


/* Open a connection to the memmap device */
static int memmap_dev_open(const uintptr_t dev_spec __unused,
			   io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&memmap_dev_info; /* cast away const */

	return 0;
}



/* Close a connection to the memmap device */
static int memmap_dev_close(io_dev_info_t *dev_info)
{
	/* NOP */
	/* TODO: Consider tracking open files and cleaning them up here */
	return 0;
}


/* Open a file on the memmap device */
static int memmap_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
			     io_entity_t *entity)
{
	int result = -ENOMEM;
	const io_block_spec_t *block_spec = (io_block_spec_t *)spec;

	/* Since we need to track open state for seek() we only allow one open
	 * spec at a time. When we have dynamic memory we can malloc and set
	 * entity->info.
	 */
	if (current_file.in_use == 0) {
		assert(block_spec != NULL);
		assert(entity != NULL);

		current_file.in_use = 1;
		current_file.base = block_spec->offset;
		/* File cursor offset for seek and incremental reads etc. */
		current_file.file_pos = 0;
		current_file.size = block_spec->length;
		entity->info = (uintptr_t)&current_file;
		result = 0;
	} else {
		WARN("A Memmap device is already active. Close first.\n");
	}

	return result;
}


/* Seek to a particular file offset on the memmap device */
static int memmap_block_seek(io_entity_t *entity, int mode,
			     signed long long offset)
{
	int result = -ENOENT;
	file_state_t *fp;

	/* We only support IO_SEEK_SET for the moment. */
	if (mode == IO_SEEK_SET) {
		assert(entity != NULL);

		fp = (file_state_t *) entity->info;

		/* Assert that new file position is valid */
		assert((offset >= 0) &&
		       ((unsigned long long)offset < fp->size));

		/* Reset file position */
		fp->file_pos = (unsigned long long)offset;
		result = 0;
	}

	return result;
}


/* Return the size of a file on the memmap device */
static int memmap_block_len(io_entity_t *entity, size_t *length)
{
	assert(entity != NULL);
	assert(length != NULL);

	*length = (size_t)((file_state_t *)entity->info)->size;

	return 0;
}


/* Read data from a file on the memmap device */
static int memmap_block_read(io_entity_t *entity, uintptr_t buffer,
			     size_t length, size_t *length_read)
{
	file_state_t *fp;
	unsigned long long pos_after;

	assert(entity != NULL);
	assert(length_read != NULL);

	fp = (file_state_t *) entity->info;

	/* Assert that file position is valid for this read operation */
	pos_after = fp->file_pos + length;
	assert((pos_after >= fp->file_pos) && (pos_after <= fp->size));

	memcpy((void *)buffer,
	       (void *)((uintptr_t)(fp->base + fp->file_pos)), length);

	*length_read = length;

	/* Set file position after read */
	fp->file_pos = pos_after;

	return 0;
}


/* Write data to a file on the memmap device */
static int memmap_block_write(io_entity_t *entity, const uintptr_t buffer,
			      size_t length, size_t *length_written)
{
	file_state_t *fp;
	unsigned long long pos_after;

	assert(entity != NULL);
	assert(length_written != NULL);

	fp = (file_state_t *) entity->info;

	/* Assert that file position is valid for this write operation */
	pos_after = fp->file_pos + length;
	assert((pos_after >= fp->file_pos) && (pos_after <= fp->size));

	memcpy((void *)((uintptr_t)(fp->base + fp->file_pos)),
	       (void *)buffer, length);

	*length_written = length;

	/* Set file position after write */
	fp->file_pos = pos_after;

	return 0;
}


/* Close a file on the memmap device */
static int memmap_block_close(io_entity_t *entity)
{
	assert(entity != NULL);

	entity->info = 0;

	/* This would be a mem free() if we had malloc.*/
	zeromem((void *)&current_file, sizeof(current_file));

	return 0;
}


/* Exported functions */

/* Register the memmap driver with the IO abstraction */
int register_io_dev_memmap(const io_dev_connector_t **dev_con)
{
	int result;
	assert(dev_con != NULL);

	result = io_register_device(&memmap_dev_info);
	if (result == 0)
		*dev_con = &memmap_dev_connector;

	return result;
}
