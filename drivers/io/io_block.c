/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>
#include <lib/utils.h>

typedef struct {
	io_block_dev_spec_t	*dev_spec;
	uintptr_t		base;
	unsigned long long	file_pos;
	unsigned long long	size;
} block_dev_state_t;

#define is_power_of_2(x)	(((x) != 0U) && (((x) & ((x) - 1U)) == 0U))

io_type_t device_type_block(void);

static int block_open(io_dev_info_t *dev_info, const uintptr_t spec,
		      io_entity_t *entity);
static int block_seek(io_entity_t *entity, int mode, signed long long offset);
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
	unsigned int index;
	int result = -ENOENT;

	for (index = 0U; index < MAX_IO_BLOCK_DEVICES; ++index) {
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
		zeromem(state, sizeof(block_dev_state_t));
		zeromem(dev_info, sizeof(io_dev_info_t));
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
static int block_seek(io_entity_t *entity, int mode, signed long long offset)
{
	block_dev_state_t *cur;

	assert(entity->info != (uintptr_t)NULL);

	cur = (block_dev_state_t *)entity->info;
	assert((offset >= 0) && ((unsigned long long)offset < cur->size));

	switch (mode) {
	case IO_SEEK_SET:
		cur->file_pos = (unsigned long long)offset;
		break;
	case IO_SEEK_CUR:
		cur->file_pos += (unsigned long long)offset;
		break;
	default:
		return -EINVAL;
	}
	assert(cur->file_pos < cur->size);
	return 0;
}

/*
 * This function allows the caller to read any number of bytes
 * from any position. It hides from the caller that the low level
 * driver only can read aligned blocks of data. For this reason
 * we need to handle the use case where the first byte to be read is not
 * aligned to start of the block, the last byte to be read is also not
 * aligned to the end of a block, and there are zero or more blocks-worth
 * of data in between.
 *
 * In such a case we need to read more bytes than requested (i.e. full
 * blocks) and strip-out the leading bytes (aka skip) and the trailing
 * bytes (aka padding). See diagram below
 *
 * cur->file_pos ------------
 *                          |
 * cur->base                |
 *  |                       |
 *  v                       v<----  length   ---->
 *  --------------------------------------------------------------
 * |           |         block#1    |        |   block#n          |
 * |  block#0  |            +       |   ...  |     +              |
 * |           | <- skip -> +       |        |     + <- padding ->|
 *  ------------------------+----------------------+--------------
 *             ^                                                  ^
 *             |                                                  |
 *             v    iteration#1                iteration#n        v
 *              --------------------------------------------------
 *             |                    |        |                    |
 *             |<----  request ---->|  ...   |<----- request ---->|
 *             |                    |        |                    |
 *              --------------------------------------------------
 *            /                   /          |                    |
 *           /                   /           |                    |
 *          /                   /            |                    |
 *         /                   /             |                    |
 *        /                   /              |                    |
 *       /                   /               |                    |
 *      /                   /                |                    |
 *     /                   /                 |                    |
 *    /                   /                  |                    |
 *   /                   /                   |                    |
 *  <---- request ------>                    <------ request  ----->
 *  ---------------------                    -----------------------
 *  |        |          |                    |          |           |
 *  |<-skip->|<-nbytes->|           -------->|<-nbytes->|<-padding->|
 *  |        |          |           |        |          |           |
 *  ---------------------           |        -----------------------
 *  ^        \           \          |        |          |
 *  |         \           \         |        |          |
 *  |          \           \        |        |          |
 *  buf->offset \           \   buf->offset  |          |
 *               \           \               |          |
 *                \           \              |          |
 *                 \           \             |          |
 *                  \           \            |          |
 *                   \           \           |          |
 *                    \           \          |          |
 *                     \           \         |          |
 *                      --------------------------------
 *                      |           |        |         |
 * buffer-------------->|           | ...    |         |
 *                      |           |        |         |
 *                      --------------------------------
 *                      <-count#1->|                   |
 *                      <----------  count#n   -------->
 *                      <----------  length  ---------->
 *
 * Additionally, the IO driver has an underlying buffer that is at least
 * one block-size and may be big enough to allow.
 */
static int block_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		      size_t *length_read)
{
	block_dev_state_t *cur;
	io_block_spec_t *buf;
	io_block_ops_t *ops;
	int lba;
	size_t block_size, left;
	size_t nbytes;  /* number of bytes read in one iteration */
	size_t request; /* number of requested bytes in one iteration */
	size_t count;   /* number of bytes already read */
	/*
	 * number of leading bytes from start of the block
	 * to the first byte to be read
	 */
	size_t skip;

	/*
	 * number of trailing bytes between the last byte
	 * to be read and the end of the block
	 */
	size_t padding;

	assert(entity->info != (uintptr_t)NULL);
	cur = (block_dev_state_t *)entity->info;
	ops = &(cur->dev_spec->ops);
	buf = &(cur->dev_spec->buffer);
	block_size = cur->dev_spec->block_size;
	assert((length <= cur->size) &&
	       (length > 0U) &&
	       (ops->read != 0));

	/*
	 * We don't know the number of bytes that we are going
	 * to read in every iteration, because it will depend
	 * on the low level driver.
	 */
	count = 0;
	for (left = length; left > 0U; left -= nbytes) {
		/*
		 * We must only request operations aligned to the block
		 * size. Therefore if file_pos is not block-aligned,
		 * we have to request the operation to start at the
		 * previous block boundary and skip the leading bytes. And
		 * similarly, the number of bytes requested must be a
		 * block size multiple
		 */
		skip = cur->file_pos & (block_size - 1U);

		/*
		 * Calculate the block number containing file_pos
		 * - e.g. block 3.
		 */
		lba = (cur->file_pos + cur->base) / block_size;

		if ((skip + left) > buf->length) {
			/*
			 * The underlying read buffer is too small to
			 * read all the required data - limit to just
			 * fill the buffer, and then read again.
			 */
			request = buf->length;
		} else {
			/*
			 * The underlying read buffer is big enough to
			 * read all the required data. Calculate the
			 * number of bytes to read to align with the
			 * block size.
			 */
			request = skip + left;
			request = (request + (block_size - 1U)) &
				~(block_size - 1U);
		}
		request = ops->read(lba, buf->offset, request);

		if (request <= skip) {
			/*
			 * We couldn't read enough bytes to jump over
			 * the skip bytes, so we should have to read
			 * again the same block, thus generating
			 * the same error.
			 */
			return -EIO;
		}

		/*
		 * Need to remove skip and padding bytes,if any, from
		 * the read data when copying to the user buffer.
		 */
		nbytes = request - skip;
		padding = (nbytes > left) ? nbytes - left : 0U;
		nbytes -= padding;

		memcpy((void *)(buffer + count),
		       (void *)(buf->offset + skip),
		       nbytes);

		cur->file_pos += nbytes;
		count += nbytes;
	}
	assert(count == length);
	*length_read = count;

	return 0;
}

/*
 * This function allows the caller to write any number of bytes
 * from any position. It hides from the caller that the low level
 * driver only can write aligned blocks of data.
 * See comments for block_read for more details.
 */
static int block_write(io_entity_t *entity, const uintptr_t buffer,
		       size_t length, size_t *length_written)
{
	block_dev_state_t *cur;
	io_block_spec_t *buf;
	io_block_ops_t *ops;
	int lba;
	size_t block_size, left;
	size_t nbytes;  /* number of bytes read in one iteration */
	size_t request; /* number of requested bytes in one iteration */
	size_t count;   /* number of bytes already read */
	/*
	 * number of leading bytes from start of the block
	 * to the first byte to be read
	 */
	size_t skip;

	/*
	 * number of trailing bytes between the last byte
	 * to be read and the end of the block
	 */
	size_t padding;

	assert(entity->info != (uintptr_t)NULL);
	cur = (block_dev_state_t *)entity->info;
	ops = &(cur->dev_spec->ops);
	buf = &(cur->dev_spec->buffer);
	block_size = cur->dev_spec->block_size;
	assert((length <= cur->size) &&
	       (length > 0U) &&
	       (ops->read != 0) &&
	       (ops->write != 0));

	/*
	 * We don't know the number of bytes that we are going
	 * to write in every iteration, because it will depend
	 * on the low level driver.
	 */
	count = 0;
	for (left = length; left > 0U; left -= nbytes) {
		/*
		 * We must only request operations aligned to the block
		 * size. Therefore if file_pos is not block-aligned,
		 * we have to request the operation to start at the
		 * previous block boundary and skip the leading bytes. And
		 * similarly, the number of bytes requested must be a
		 * block size multiple
		 */
		skip = cur->file_pos & (block_size - 1U);

		/*
		 * Calculate the block number containing file_pos
		 * - e.g. block 3.
		 */
		lba = (cur->file_pos + cur->base) / block_size;

		if ((skip + left) > buf->length) {
			/*
			 * The underlying read buffer is too small to
			 * read all the required data - limit to just
			 * fill the buffer, and then read again.
			 */
			request = buf->length;
		} else {
			/*
			 * The underlying read buffer is big enough to
			 * read all the required data. Calculate the
			 * number of bytes to read to align with the
			 * block size.
			 */
			request = skip + left;
			request = (request + (block_size - 1U)) &
				~(block_size - 1U);
		}

		/*
		 * The number of bytes that we are going to write
		 * from the user buffer will depend of the size
		 * of the current request.
		 */
		nbytes = request - skip;
		padding = (nbytes > left) ? nbytes - left : 0U;
		nbytes -= padding;

		/*
		 * If we have skip or padding bytes then we have to preserve
		 * some content and it means that we have to read before
		 * writing
		 */
		if ((skip > 0U) || (padding > 0U)) {
			request = ops->read(lba, buf->offset, request);
			/*
			 * The read may return size less than
			 * requested. Round down to the nearest block
			 * boundary
			 */
			request &= ~(block_size - 1U);
			if (request <= skip) {
				/*
				 * We couldn't read enough bytes to jump over
				 * the skip bytes, so we should have to read
				 * again the same block, thus generating
				 * the same error.
				 */
				return -EIO;
			}
			nbytes = request - skip;
			padding = (nbytes > left) ? nbytes - left : 0U;
			nbytes -= padding;
		}

		memcpy((void *)(buf->offset + skip),
		       (void *)(buffer + count),
		       nbytes);

		request = ops->write(lba, buf->offset, request);
		if (request <= skip)
			return -EIO;

		/*
		 * And the previous write operation may modify the size
		 * of the request, so again, we have to calculate the
		 * number of bytes that we consumed from the user
		 * buffer
		 */
		nbytes = request - skip;
		padding = (nbytes > left) ? nbytes - left : 0U;
		nbytes -= padding;

		cur->file_pos += nbytes;
		count += nbytes;
	}
	assert(count == length);
	*length_written = count;

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
	if (result != 0)
		return -ENOENT;

	cur = (block_dev_state_t *)info->info;
	/* dev_spec is type of io_block_dev_spec_t. */
	cur->dev_spec = (io_block_dev_spec_t *)dev_spec;
	buffer = &(cur->dev_spec->buffer);
	block_size = cur->dev_spec->block_size;
	assert((block_size > 0U) &&
	       (is_power_of_2(block_size) != 0U) &&
	       ((buffer->offset % block_size) == 0U) &&
	       ((buffer->length % block_size) == 0U));

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
