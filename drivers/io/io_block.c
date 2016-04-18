/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <io_block.h>
#include <io_driver.h>
#include <io_storage.h>
#include <string.h>

#define MIN_BLOCK_SIZE			64
#define MIN_BLOCK_MASK			(MIN_BLOCK_SIZE - 1)

io_type_t device_type_block(void);

static int block_open(io_dev_info_t *dev_info, const uintptr_t spec,
		      io_entity_t *entity);
static int block_seek(io_entity_t *entity, int mode, ssize_t offset);
static int block_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		      size_t *length_read);
static int block_write(io_entity_t *entity, uintptr_t buffer, size_t length,
		       size_t *length_written);
static int block_close(io_entity_t *entity);
static int block_dev_init(io_dev_info_t *dev_info,
			  const uintptr_t init_params);
static int block_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int block_dev_close(io_dev_info_t *dev_info);

static const io_dev_connector_t block_dev_connector = {
	.dev_open	= block_dev_open
};

static const io_dev_funcs_t block_dev_funcs = {
	.type		= device_type_block,
	.open		= block_open,
	.seek		= block_seek,
	.size		= NULL,
	.read		= block_read,
	.write		= block_write,
	.close		= block_close,
	.dev_init	= block_dev_init,
	.dev_close	= block_dev_close,
};

static io_dev_info_t block_dev_info = {
	.funcs	= &block_dev_funcs,
	.info	= (uintptr_t)NULL,
};

io_type_t device_type_block(void)
{
	return IO_TYPE_BLOCK;
}

static int block_open(io_dev_info_t *dev_info, const uintptr_t spec,
		      io_entity_t *entity)
{
	io_block_entity_t *block;
	io_block_spec_t *region;

	assert((dev_info->info != (uintptr_t)NULL) &&
	       (spec != (uintptr_t)NULL) &&
	       (entity->info == (uintptr_t)NULL));

	block = (io_block_entity_t *)spec;
	region = &block->region;
	memcpy((void *)&block->buffer, (void *)dev_info->info,
	       sizeof(io_block_spec_t));

	assert((block->block_size > 0) &&
	       ((block->block_size & MIN_BLOCK_MASK) == 0) &&
	       ((region->offset % block->block_size) == 0) &&
	       ((region->length % block->block_size) == 0) &&
	       ((block->buffer.offset % block->block_size) == 0) &&
	       ((block->buffer.length % block->block_size) == 0));

	block->file_pos = 0;
	block->lba = (block->file_pos + region->offset) / block->block_size;
	entity->info = spec;
	return 0;
}

/* parameter offset is relative address at here */
static int block_seek(io_entity_t *entity, int mode, ssize_t offset)
{
	io_block_entity_t *block;
	io_block_spec_t *region;

	assert(entity->info != (uintptr_t)NULL);

	block = (io_block_entity_t *)entity->info;
	region = &block->region;

	assert(offset < block->region.length);

	block->file_pos += offset;
	block->lba = (block->file_pos + region->offset) / block->block_size;
	assert(block->lba >= 0);
	return 0;
}

static int block_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		      size_t *length_read)
{
	io_block_entity_t *block;
	io_block_spec_t *region, *buf;
	size_t aligned_length, skip, count, left, padding;

	assert(entity->info != (uintptr_t)NULL);
	block = (io_block_entity_t *)entity->info;
	region = &block->region;
	assert((length <= region->length) &&
	       (length > 0) &&
	       (block->ops.read != 0));

	buf = &block->buffer;
	skip = block->file_pos % block->block_size;
	aligned_length = ((skip + length) + (block->block_size - 1)) &
			 ~(block->block_size - 1);
	padding = aligned_length - (skip + length);
	left = aligned_length;
	do {
		if (left >= buf->length) {
			/* Since left is larger, it's impossible to padding. */
			if (skip)
				count = block->ops.read(block->lba, buf->offset,
							buf->length);
			else
				count = block->ops.read(block->lba, buffer,
							buf->length);
			assert(count == buf->length);
			block->file_pos += count - skip;
			if (skip) {
				memcpy((void *)buffer,
				       (void *)(buf->offset + skip),
				       count - skip);
			}
			left = left - (count - skip);
		} else {
			if (skip || padding)
				count = block->ops.read(block->lba, buf->offset,
							left);
			else
				count = block->ops.read(block->lba, buffer,
							left);
			assert(count == left);
			left = left - (skip + padding);
			block->file_pos += left;
			if (skip || padding) {
				memcpy((void *)buffer,
				       (void *)(buf->offset + skip),
				       left);
			}
			left = 0;
		}
		block->lba = (block->file_pos + region->offset) /
			     block->block_size;
		skip = block->file_pos % block->block_size;
	} while (left > 0);
	*length_read = length;

	return 0;
}

static int block_write(io_entity_t *entity, uintptr_t buffer, size_t length,
		       size_t *length_written)
{
	io_block_entity_t *block;
	io_block_spec_t *region, *buf;
	size_t aligned_length, skip, count, left, padding;

	assert(entity->info != (uintptr_t)NULL);
	block = (io_block_entity_t *)entity->info;
	region = &block->region;
	assert((length <= region->length) &&
	       (length > 0) &&
	       (block->ops.read != 0));

	buf = &block->buffer;
	skip = block->file_pos % block->block_size;
	aligned_length = ((skip + length) + (block->block_size - 1)) &
			 ~(block->block_size - 1);
	padding = aligned_length - (skip + length);
	left = aligned_length;
	do {
		if (left >= buf->length) {
			/* Since left is larger, it's impossible to padding. */
			if (skip)
				count = block->ops.write(block->lba,
							 buf->offset,
							 buf->length);
			else
				count = block->ops.write(block->lba, buffer,
							 buf->length);
			assert(count == buf->length);
			block->file_pos += count - skip;
			if (skip) {
				memcpy((void *)buffer,
				       (void *)(buf->offset + skip),
				       count - skip);
			}
			left = left - (count - skip);
		} else {
			if (skip || padding)
				count = block->ops.write(block->lba,
							 buf->offset,
							 left);
			else
				count = block->ops.write(block->lba, buffer,
							 left);
			assert(count == left);
			left = left - (skip + padding);
			block->file_pos += left;
			if (skip || padding) {
				memcpy((void *)buffer,
				       (void *)(buf->offset + skip),
				       left);
			}
			left = 0;
		}
		block->lba = (block->file_pos + region->offset) /
			     block->block_size;
		skip = block->file_pos % block->block_size;
	} while (left > 0);
	*length_written = length;
	return 0;
}

static int block_close(io_entity_t *entity)
{
	entity->info = (uintptr_t)NULL;
	return 0;
}

static int block_dev_init(io_dev_info_t *dev_info,
			  const uintptr_t init_params)
{
	assert((dev_info != NULL) && (dev_info->info != 0));

	return 0;
}

static int block_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);

	*dev_info = (io_dev_info_t *)&block_dev_info;	/* cast away const */
	(*dev_info)->info = dev_spec;
	return 0;
}

static int block_dev_close(io_dev_info_t *dev_info)
{
	dev_info->info = (uintptr_t)NULL;
	return 0;
}

/* Exported functions */

/* Register the Block driver with the IO abstraction */
int register_io_dev_block(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);
	result = io_register_device(&block_dev_info);
	if (result == 0)
		*dev_con = &block_dev_connector;
	return result;
}
