/*
 * Copyright (c) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fw_cfg.h>
#include <drivers/io/io_storage.h>
#include <drivers/qemu/qemu_fw_cfg.h>
#include <lib/utils.h>

typedef struct {
	bool in_use;
	uint16_t select;
	size_t file_pos;
	size_t size;
} fw_cfg_file_state_t;

static fw_cfg_file_state_t current_file;

static io_type_t device_type_fw_cfg(void)
{
	return IO_TYPE_FW_CFG;
}

static int fw_cfg_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int fw_cfg_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
			    io_entity_t *entity);
static int fw_cfg_file_seek(io_entity_t *entity, int mode,
			    signed long long offset);
static int fw_cfg_file_len(io_entity_t *entity, size_t *length);
static int fw_cfg_file_read(io_entity_t *entity, uintptr_t buffer,
			    size_t length, size_t *length_read);
static int fw_cfg_file_close(io_entity_t *entity);

static const io_dev_connector_t fw_cfg_dev_connector = {
	.dev_open = fw_cfg_dev_open
};

static const io_dev_funcs_t fw_cfg_dev_funcs = {
	.type = device_type_fw_cfg,
	.open = fw_cfg_file_open,
	.seek = fw_cfg_file_seek,
	.size = fw_cfg_file_len,
	.read = fw_cfg_file_read,
	.write = NULL,
	.close = fw_cfg_file_close,
	.dev_init = NULL,
	.dev_close = NULL,
};

static io_dev_info_t fw_cfg_dev_info = {
	.funcs = &fw_cfg_dev_funcs,
	.info = (uintptr_t)NULL
};

static int fw_cfg_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info)
{
	int result;

	assert(dev_info != NULL);
	assert(dev_spec != (uintptr_t)NULL);

	result = qemu_fw_cfg_init(dev_spec);
	if (result != 0) {
		return result;
	}

	*dev_info = &fw_cfg_dev_info;

	return 0;
}

static int fw_cfg_file_open(io_dev_info_t *dev_info __unused,
			    const uintptr_t spec, io_entity_t *entity)
{
	const io_file_spec_t *file_spec = (const io_file_spec_t *)spec;
	int result;

	assert(file_spec != NULL);
	assert(file_spec->path != NULL);
	assert(entity != NULL);

	if (current_file.in_use) {
		WARN("fw_cfg: Only one open file at a time.\n");
		return -ENOMEM;
	}

	result = qemu_fw_cfg_find_file(file_spec->path, &current_file.select,
				       &current_file.size);
	if (result == 0) {
		current_file.in_use = true;
		current_file.file_pos = 0U;
		qemu_fw_cfg_select(current_file.select);
		entity->info = (uintptr_t)&current_file;
	}

	return result;
}

static int fw_cfg_file_seek(io_entity_t *entity, int mode,
			    signed long long offset)
{
	fw_cfg_file_state_t *fp;

	if (mode != IO_SEEK_SET) {
		return -EINVAL;
	}

	assert(entity != NULL);
	fp = (fw_cfg_file_state_t *)entity->info;

	if ((offset < 0) || ((size_t)offset > fp->size)) {
		return -EINVAL;
	}

	if ((size_t)offset < fp->file_pos) {
		qemu_fw_cfg_select(fp->select);
		fp->file_pos = 0U;
	}

	qemu_fw_cfg_skip_bytes((size_t)offset - fp->file_pos);
	fp->file_pos = (size_t)offset;

	return 0;
}

static int fw_cfg_file_len(io_entity_t *entity, size_t *length)
{
	assert(entity != NULL);
	assert(length != NULL);

	*length = ((fw_cfg_file_state_t *)entity->info)->size;

	return 0;
}

static int fw_cfg_file_read(io_entity_t *entity, uintptr_t buffer,
			    size_t length, size_t *length_read)
{
	fw_cfg_file_state_t *fp;
	size_t pos_after;

	assert(entity != NULL);
	assert(length_read != NULL);

	fp = (fw_cfg_file_state_t *)entity->info;

	pos_after = fp->file_pos + length;
	if ((pos_after < fp->file_pos) || (pos_after > fp->size)) {
		return -EINVAL;
	}

	qemu_fw_cfg_read_bytes((uint8_t *)buffer, length);
	fp->file_pos = pos_after;
	*length_read = length;

	return 0;
}

static int fw_cfg_file_close(io_entity_t *entity)
{
	assert(entity != NULL);

	entity->info = 0U;
	zeromem(&current_file, sizeof(current_file));

	return 0;
}

int register_io_dev_fw_cfg(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);

	result = io_register_device(&fw_cfg_dev_info);
	if (result == 0) {
		*dev_con = &fw_cfg_dev_connector;
	}

	return result;
}
