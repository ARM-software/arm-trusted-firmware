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
#include <platform_def.h>
#include <string.h>

typedef struct {
	io_block_dev_spec_t	*dev_spec;
	uintptr_t		base;
	size_t			file_pos;
	size_t			size;
} block_dev_state_t;

#define is_power_of_2(x)	((x != 0) && ((x & (x - 1)) == 0))

io_type_t device_type_block(void);

static int block_open(io_dev_info_t *dev_info, const uintptr_t spec,
		      io_entity_t *entity);
static int block_seek(io_entity_t *entity, int mode, ssize_t offset);
static int block_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		      size_t *length_read);
static int block_write(io_entity_t *entity, const uintptr_t buffer,
		       size_t length, size_t *length_written);
static int block_close(io_entity_t *entity);
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
	.dev_init	= NULL,
	.dev_close	= block_dev_close,
};

static block_dev_state_t state_pool[MAX_IO_BLOCK_DEVICES];
static io_dev_info_t dev_info_pool[MAX_IO_BLOCK_DEVICES];

/* Track number of allocated block state */
static unsigned int block_dev_count;

io_type_t device_type_block(void)
{
	return IO_TYPE_BLOCK;
}

/* Locate a block state in the pool, specified by address */
static int find_first_block_state(const io_block_dev_spec_t *dev_spec,
				  unsigned int *index_out)
{
	int result = -ENOENT;
	for (int index = 0; index < MAX_IO_BLOCK_DEVICES; ++index) {
		/* dev_spec is used as identifier since it's unique */
		if (state_pool[index].dev_spec == dev_spec) {
			result = 0;
			*index_out = index;
			break;
		}
	}
	return result;
}

/* Allocate a device info from the pool and return a pointer to it */
static int allocate_dev_info(io_dev_info_t **dev_info)
{
	int result = -ENOMEM;
	assert(dev_info != NULL);

	if (block_dev_count < MAX_IO_BLOCK_DEVICES) {
		unsigned int index = 0;
		result = find_first_block_state(NULL, &index);
		assert(result == 0);
		/* initialize dev_info */
		dev_info_pool[index].funcs = &block_dev_funcs;
		dev_info_pool[index].info = (uintptr_t)&state_pool[index];
		*dev_info = &dev_info_pool[index];
		++block_dev_count;
	}

	return result;
}


/* Release a device info to the pool */
static int free_dev_info(io_dev_info_t *dev_info)
{
	int result;
	unsigned int index = 0;
	block_dev_state_t *state;
	assert(dev_info != NULL);

	state = (block_dev_state_t *)dev_info->info;
	result = find_first_block_state(state->dev_spec, &index);
	if (result ==  0) {
		/* free if device info is valid */
		memset(state, 0, sizeof(block_dev_state_t));
		memset(dev_info, 0, sizeof(io_dev_info_t));
		--block_dev_count;
	}

	return result;
}

static int block_open(io_dev_info_t *dev_info, const uintptr_t spec,
		      io_entity_t *entity)
{
	block_dev_state_t *cur;
	io_block_spec_t *region;

	assert((dev_info->info != (uintptr_t)NULL) &&
	       (spec != (uintptr_t)NULL) &&
	       (entity->info == (uintptr_t)NULL));

	region = (io_block_spec_t *)spec;
	cur = (block_dev_state_t *)dev_info->info;
	assert(((region->offset % cur->dev_spec->block_size) == 0) &&
	       ((region->length % cur->dev_spec->block_size) == 0));

	cur->base = region->offset;
	cur->size = region->length;
	cur->file_pos = 0;

	entity->info = (uintptr_t)cur;
	return 0;
}

/* parameter offset is relative address at here */
static int block_seek(io_entity_t *entity, int mode, ssize_t offset)
{
	block_dev_state_t *cur;

	assert(entity->info != (uintptr_t)NULL);

	cur = (block_dev_state_t *)entity->info;
	assert((offset >= 0) && (offset < cur->size));

	switch (mode) {
	case IO_SEEK_SET:
		cur->file_pos = offset;
		break;
	case IO_SEEK_CUR:
		cur->file_pos += offset;
		break;
	default:
		return -EINVAL;
	}
	assert(cur->file_pos < cur->size);
	return 0;
}

static int block_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		      size_t *length_read)
{
	block_dev_state_t *cur;
	io_block_spec_t *buf;
	io_block_ops_t *ops;
	size_t aligned_length, skip, count, left, padding, block_size;
	int lba;
	int buffer_not_aligned;

	assert(entity->info != (uintptr_t)NULL);
	cur = (block_dev_state_t *)entity->info;
	ops = &(cur->dev_spec->ops);
	buf = &(cur->dev_spec->buffer);
	block_size = cur->dev_spec->block_size;
	assert((length <= cur->size) &&
	       (length > 0) &&
	       (ops->read != 0));

	if ((buffer & (block_size - 1)) != 0) {
		/*
		 * buffer isn't aligned with block size.
		 * Block device always relies on DMA operation.
		 * It's better to make the buffer as block size aligned.
		 */
		buffer_not_aligned = 1;
	} else {
		buffer_not_aligned = 0;
	}

	skip = cur->file_pos % block_size;
	aligned_length = ((skip + length) + (block_size - 1)) &
			 ~(block_size - 1);
	padding = aligned_length - (skip + length);
	left = aligned_length;
	do {
		lba = (cur->file_pos + cur->base) / block_size;
		if (left >= buf->length) {
			/*
			 * Since left is larger, it's impossible to padding.
			 *
			 * If buffer isn't aligned, we need to use aligned
			 * buffer instead.
			 */
			if (skip || buffer_not_aligned) {
				/*
				 * The beginning address (file_pos) isn't
				 * aligned with block size, we need to use
				 * block buffer to read block. Since block
				 * device is always relied on DMA operation.
				 */
				count = ops->read(lba, buf->offset,
						  buf->length);
			} else {
				count = ops->read(lba, buffer, buf->length);
			}
			assert(count == buf->length);
			cur->file_pos += count - skip;
			if (skip || buffer_not_aligned) {
				/*
				 * Since there's not aligned block size caused
				 * by skip or not aligned buffer, block buffer
				 * is used to store data.
				 */
				memcpy((void *)buffer,
				       (void *)(buf->offset + skip),
				       count - skip);
			}
			left = left - (count - skip);
		} else {
			if (skip || padding || buffer_not_aligned) {
				/*
				 * The beginning address (file_pos) isn't
				 * aligned with block size, we have to read
				 * full block by block buffer instead.
				 * The size isn't aligned with block size.
				 * Use block buffer to avoid overflow.
				 *
				 * If buffer isn't aligned, use block buffer
				 * to avoid DMA error.
				 */
				count = ops->read(lba, buf->offset, left);
			} else
				count = ops->read(lba, buffer, left);
			assert(count == left);
			left = left - (skip + padding);
			cur->file_pos += left;
			if (skip || padding || buffer_not_aligned) {
				/*
				 * Since there's not aligned block size or
				 * buffer, block buffer is used to store data.
				 */
				memcpy((void *)buffer,
				       (void *)(buf->offset + skip),
				       left);
			}
			/* It's already the last block operation */
			left = 0;
		}
		skip = cur->file_pos % block_size;
	} while (left > 0);
	*length_read = length;

	return 0;
}

static int block_write(io_entity_t *entity, const uintptr_t buffer,
		       size_t length, size_t *length_written)
{
	block_dev_state_t *cur;
	io_block_spec_t *buf;
	io_block_ops_t *ops;
	size_t aligned_length, skip, count, left, padding, block_size;
	int lba;
	int buffer_not_aligned;

	assert(entity->info != (uintptr_t)NULL);
	cur = (block_dev_state_t *)entity->info;
	ops = &(cur->dev_spec->ops);
	buf = &(cur->dev_spec->buffer);
	block_size = cur->dev_spec->block_size;
	assert((length <= cur->size) &&
	       (length > 0) &&
	       (ops->read != 0) &&
	       (ops->write != 0));

	if ((buffer & (block_size - 1)) != 0) {
		/*
		 * buffer isn't aligned with block size.
		 * Block device always relies on DMA operation.
		 * It's better to make the buffer as block size aligned.
		 */
		buffer_not_aligned = 1;
	} else {
		buffer_not_aligned = 0;
	}

	skip = cur->file_pos % block_size;
	aligned_length = ((skip + length) + (block_size - 1)) &
			 ~(block_size - 1);
	padding = aligned_length - (skip + length);
	left = aligned_length;
	do {
		lba = (cur->file_pos + cur->base) / block_size;
		if (left >= buf->length) {
			/* Since left is larger, it's impossible to padding. */
			if (skip || buffer_not_aligned) {
				/*
				 * The beginning address (file_pos) isn't
				 * aligned with block size or buffer isn't
				 * aligned, we need to use block buffer to
				 * write block.
				 */
				count = ops->read(lba, buf->offset,
						  buf->length);
				assert(count == buf->length);
				memcpy((void *)(buf->offset + skip),
				       (void *)buffer,
				       count - skip);
				count = ops->write(lba, buf->offset,
						   buf->length);
			} else
				count = ops->write(lba, buffer, buf->length);
			assert(count == buf->length);
			cur->file_pos += count - skip;
			left = left - (count - skip);
		} else {
			if (skip || padding || buffer_not_aligned) {
				/*
				 * The beginning address (file_pos) isn't
				 * aligned with block size, we need to avoid
				 * poluate data in the beginning. Reading and
				 * skipping the beginning is the only way.
				 * The size isn't aligned with block size.
				 * Use block buffer to avoid overflow.
				 *
				 * If buffer isn't aligned, use block buffer
				 * to avoid DMA error.
				 */
				count = ops->read(lba, buf->offset, left);
				assert(count == left);
				memcpy((void *)(buf->offset + skip),
				       (void *)buffer,
				       left - skip - padding);
				count = ops->write(lba, buf->offset, left);
			} else
				count = ops->write(lba, buffer, left);
			assert(count == left);
			cur->file_pos += left - (skip + padding);
			/* It's already the last block operation */
			left = 0;
		}
		skip = cur->file_pos % block_size;
	} while (left > 0);
	*length_written = length;
	return 0;
}

static int block_close(io_entity_t *entity)
{
	entity->info = (uintptr_t)NULL;
	return 0;
}

static int block_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info)
{
	block_dev_state_t *cur;
	io_block_spec_t *buffer;
	io_dev_info_t *info;
	size_t block_size;
	int result;

	assert(dev_info != NULL);
	result = allocate_dev_info(&info);
	if (result)
		return -ENOENT;

	cur = (block_dev_state_t *)info->info;
	/* dev_spec is type of io_block_dev_spec_t. */
	cur->dev_spec = (io_block_dev_spec_t *)dev_spec;
	buffer = &(cur->dev_spec->buffer);
	block_size = cur->dev_spec->block_size;
	assert((block_size > 0) &&
	       (is_power_of_2(block_size) != 0) &&
	       ((buffer->offset % block_size) == 0) &&
	       ((buffer->length % block_size) == 0));

	*dev_info = info;	/* cast away const */
	(void)block_size;
	(void)buffer;
	return 0;
}

static int block_dev_close(io_dev_info_t *dev_info)
{
	return free_dev_info(dev_info);
}

/* Exported functions */

/* Register the Block driver with the IO abstraction */
int register_io_dev_block(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);

	/*
	 * Since dev_info isn't really used in io_register_device, always
	 * use the same device info at here instead.
	 */
	result = io_register_device(&dev_info_pool[0]);
	if (result == 0)
		*dev_con = &block_dev_connector;
	return result;
}
