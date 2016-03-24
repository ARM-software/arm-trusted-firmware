/*
 *    Copyright 2016 Broadcom
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <debug.h>
#include <io_driver.h>
#include <io_storage.h>
#include <string.h>
#include <mmio.h>
#include <emmc.h>

/*
 * As we need to be able to keep state for seek, only one file can be open
 * at a time. Make this a structure and point to the entity->info. When we
 * can malloc memory we can change this to support more open files.
 */
typedef struct {
	/*
	 * Use the 'in_use' flag as any value for base and file_pos could be
	 * valid.
	 */
	int in_use;
	uintptr_t base;
	size_t file_pos;
} file_state_t;

static file_state_t current_file = { 0 };

/* Identify the device type as memmap */
io_type_t device_type_emmc(void)
{
	return IO_TYPE_MEMMAP;
}

/* eMMC device functions */
static int emmc_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int emmc_dev_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
			io_entity_t *entity);
static int emmc_dev_block_seek(io_entity_t *entity, int mode, ssize_t offset);
static int emmc_dev_block_read(io_entity_t *entity, uintptr_t buffer,
			size_t length, size_t *length_read);
static int emmc_dev_block_write(io_entity_t *entity, const uintptr_t buffer,
			size_t length, size_t *length_written);
static int emmc_dev_block_close(io_entity_t *entity);
static int emmc_dev_dev_close(io_dev_info_t *dev_info);


static const io_dev_connector_t emmc_dev_connector = {
	.dev_open = emmc_dev_open
};

static const io_dev_funcs_t emmc_dev_funcs = {
	.type = device_type_emmc,
	.open = emmc_dev_block_open,
	.seek = emmc_dev_block_seek,
	.size = NULL,
	.read = emmc_dev_block_read,
	.write = emmc_dev_block_write,
	.close = emmc_dev_block_close,
	.dev_init = NULL,
	.dev_close = emmc_dev_dev_close,
};

/* No state associated with this device so structure can be const */
static const io_dev_info_t emmc_dev_info = {
	.funcs = &emmc_dev_funcs,
	.info = (uintptr_t) NULL
};


/* Open a connection to the eMMC device */
static int emmc_dev_open(const uintptr_t dev_spec __unused,
			 io_dev_info_t **dev_info)
{
	int result = -ENOMEM;
	uint32_t retv;

	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&emmc_dev_info; /* cast away const */

	retv = emmc_init();

	emmc_partition_select(EMMC_USER_AREA);

	result = retv ? 0 : -ENOENT;

	return result;
}

/* Close a connection to the eMMC device */
static int emmc_dev_dev_close(io_dev_info_t *dev_info)
{
	/* NOP */
	/* TODO: Consider tracking open files and cleaning them up here */
	return 0;
}

/* Open a file on the eMMC device */
/* TODO: Can we do any sensible limit checks on requested memory */
static int emmc_dev_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
			       io_entity_t *entity)
{
	int result = -ENOENT;
	const io_block_spec_t *block_spec = (io_block_spec_t *)spec;

	/*
	 * Since we need to track open state for seek() we only allow one open
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
		entity->info = (uintptr_t)&current_file;
		result = 0;
	} else {
		WARN("An eMMC device is already active. Close first.\n");
	}

	return result;
}

/* Seek to a particular file offset on the eMMC device */
static int emmc_dev_block_seek(io_entity_t *entity, int mode, ssize_t offset)
{
	int result = -ENOENT;

	/* We only support IO_SEEK_SET for the moment. */
	if (mode == IO_SEEK_SET) {
		assert(entity != NULL);

		/* TODO: can we do some basic limit checks on seek? */
		((file_state_t *) entity->info)->file_pos = offset;
		result = 0;
	}

	return result;
}

/* Read data from a file on the eMMC device */
static int emmc_dev_block_read(io_entity_t *entity, uintptr_t buffer,
			       size_t length, size_t *length_read)
{
	file_state_t *fp;
	uint32_t retv;
	uintptr_t emmc_addr;
	uintptr_t buffer_addr;
	size_t buffer_size;
	size_t emmc_bytes_to_read;

	assert(entity != NULL);
	assert(buffer != (uintptr_t) NULL);
	assert(length_read != NULL);

	fp = (file_state_t *) entity->info;
	emmc_addr = (uintptr_t) (fp->base + fp->file_pos);
	buffer_addr = buffer;
	buffer_size = length;
	emmc_bytes_to_read = length;

	retv = emmc_read(emmc_addr, buffer_addr,
			       buffer_size, emmc_bytes_to_read);

	if (retv)
		fp->file_pos += length;

	*length_read = ((length >> 2) + ((length % 4) ? (1) : (0))) * 4;

	return retv ? 0 : -ENOMEM;
}

/* Write data to a file on the eMMC device */
static int emmc_dev_block_write(io_entity_t *entity, const uintptr_t buffer,
				size_t length, size_t *length_written)
{
	file_state_t *fp;

	assert(entity != NULL);
	assert(buffer != (uintptr_t) NULL);
	assert(length_written != NULL);

	fp = (file_state_t *) entity->info;

	/* Currently, this driver does not allow for writes. */
	*length_written = 0;

	/* advance the file 'cursor' for incremental writes */
	fp->file_pos += length;

	return 0;
}

/* Close a file on the eMMC device */
static int emmc_dev_block_close(io_entity_t *entity)
{
	assert(entity != NULL);

	entity->info = 0;

	/* This would be a mem free() if we had malloc. */
	memset((void *)&current_file, 0, sizeof(current_file));

	return 0;
}

/* Exported functions */

/* Register the eMMC driver with the IO abstraction */
int register_io_dev_emmc(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);

	result = io_register_device(&emmc_dev_info);
	if (result == 0)
		*dev_con = &emmc_dev_connector;
	else
		INFO("register_io_dev_emmc dev_con was registered failed\n");

	return result;
}
