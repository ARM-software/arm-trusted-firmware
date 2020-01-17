/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>

#include "io_common.h"
#include "io_private.h"
#include "io_memdrv.h"
#include "rcar_def.h"

extern void rcar_dma_exec(uintptr_t dst, uint32_t src, uint32_t len);

static int32_t memdrv_dev_open(const uintptr_t dev __attribute__ ((unused)),
			       io_dev_info_t **dev_info);
static int32_t memdrv_dev_close(io_dev_info_t *dev_info);

/* As we need to be able to keep state for seek, only one file can be open
 * at a time. Make this a structure and point to the entity->info. When we
 * can malloc memory we can change this to support more open files.
 */
typedef struct {
	uint32_t in_use;
	uintptr_t base;
	signed long long file_pos;
} file_state_t;

static file_state_t current_file = { 0 };

static io_type_t device_type_memdrv(void)
{
	return IO_TYPE_MEMMAP;
}

static int32_t memdrv_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
				 io_entity_t *entity)
{
	const io_drv_spec_t *block_spec = (io_drv_spec_t *) spec;

	/* Since we need to track open state for seek() we only allow one open
	 * spec at a time. When we have dynamic memory we can malloc and set
	 * entity->info.
	 */
	if (current_file.in_use != 0U)
		return IO_RESOURCES_EXHAUSTED;

	/* File cursor offset for seek and incremental reads etc. */
	current_file.base = block_spec->offset;
	current_file.file_pos = 0;
	current_file.in_use = 1;

	entity->info = (uintptr_t) &current_file;

	return IO_SUCCESS;
}

static int32_t memdrv_block_seek(io_entity_t *entity, int32_t mode,
				 signed long long offset)
{
	if (mode != IO_SEEK_SET)
		return IO_FAIL;

	((file_state_t *) entity->info)->file_pos = offset;

	return IO_SUCCESS;
}

static int32_t memdrv_block_read(io_entity_t *entity, uintptr_t buffer,
				 size_t length, size_t *cnt)
{
	file_state_t *fp;

	fp = (file_state_t *) entity->info;

	NOTICE("BL2: dst=0x%lx src=0x%llx len=%ld(0x%lx)\n",
	       buffer, (unsigned long long)fp->base +
	       (unsigned long long)fp->file_pos, length, length);

	if (FLASH_MEMORY_SIZE < (fp->file_pos + (signed long long)length)) {
		ERROR("BL2: check load image (source address)\n");
		return IO_FAIL;
	}

	rcar_dma_exec(buffer, fp->base + (uintptr_t)fp->file_pos, length);
	fp->file_pos += (signed long long)length;
	*cnt = length;

	return IO_SUCCESS;
}

static int32_t memdrv_block_close(io_entity_t *entity)
{
	entity->info = 0U;

	memset((void *)&current_file, 0, sizeof(current_file));

	return IO_SUCCESS;
}

static const io_dev_funcs_t memdrv_dev_funcs = {
	.type = &device_type_memdrv,
	.open = &memdrv_block_open,
	.seek = &memdrv_block_seek,
	.size = NULL,
	.read = &memdrv_block_read,
	.write = NULL,
	.close = &memdrv_block_close,
	.dev_init = NULL,
	.dev_close = &memdrv_dev_close,
};

static const io_dev_info_t memdrv_dev_info = {
	.funcs = &memdrv_dev_funcs,
	.info = 0,
};

static const io_dev_connector_t memdrv_dev_connector = {
	.dev_open = &memdrv_dev_open
};

static int32_t memdrv_dev_open(const uintptr_t dev __attribute__ ((unused)),
			       io_dev_info_t **dev_info)
{
	*dev_info = (io_dev_info_t *) &memdrv_dev_info;

	return IO_SUCCESS;
}

static int32_t memdrv_dev_close(io_dev_info_t *dev_info)
{
	return IO_SUCCESS;
}

int32_t rcar_register_io_dev_memdrv(const io_dev_connector_t **dev_con)
{
	int32_t result;

	result = io_register_device(&memdrv_dev_info);
	if (result == IO_SUCCESS)
		*dev_con = &memdrv_dev_connector;

	return result;
}
