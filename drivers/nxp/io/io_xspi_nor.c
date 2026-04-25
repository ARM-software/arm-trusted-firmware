/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * io_storage backend for the NXP FlexSPI NOR controller. See
 * include/drivers/nxp/io/io_xspi_nor.h for the API contract.
 *
 * Modelled on drivers/io/io_block.c and drivers/io/io_memmap.c:
 * one io_dev_info_t, one connector, one open file at a time
 * (the underlying NOR has a single CS), per-entity offset cursor.
 *
 * The actual flash transactions are delegated to the FlexSPI NOR
 * driver in drivers/nxp/flexspi/nor/ via the public fspi_api.h
 * surface (xspi_sector_erase, xspi_write, xspi_read). This file
 * owns no flash protocol logic; it is a thin io_storage adapter.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>
#include <drivers/nxp/io/io_xspi_nor.h>
#include <fspi_api.h>
#include <lib/utils.h>

/*
 * Per-entity state. The base + length describe the region opened
 * via io_block_spec_t. file_pos is the seek cursor inside the
 * region (not into the chip).
 */
typedef struct {
	int		in_use;
	uint32_t	base;		/* flash-relative byte offset */
	uint32_t	length;		/* region size in bytes */
	uint32_t	file_pos;	/* cursor inside the region */
} xspi_nor_state_t;

static xspi_nor_state_t current_state;

static io_type_t device_type_xspi_nor(void)
{
	/*
	 * IO_TYPE_MTD is the closest existing enum value: NOR sits
	 * between block (sector-aligned random access) and MTD
	 * (erase-before-write). Reusing IO_TYPE_MTD avoids growing
	 * io_type_t for a single platform-family backend.
	 */
	return IO_TYPE_MTD;
}

static int xspi_nor_dev_open(const uintptr_t dev_spec __unused,
			     io_dev_info_t **dev_info);
static int xspi_nor_dev_close(io_dev_info_t *dev_info);
static int xspi_nor_open(io_dev_info_t *dev_info, const uintptr_t spec,
			 io_entity_t *entity);
static int xspi_nor_seek(io_entity_t *entity, int mode,
			 signed long long offset);
static int xspi_nor_size(io_entity_t *entity, size_t *length);
static int xspi_nor_read(io_entity_t *entity, uintptr_t buffer,
			 size_t length, size_t *length_read);
static int xspi_nor_write(io_entity_t *entity, const uintptr_t buffer,
			  size_t length, size_t *length_written);
static int xspi_nor_erase(io_entity_t *entity, size_t length);
static int xspi_nor_close(io_entity_t *entity);

static const io_dev_connector_t xspi_nor_connector = {
	.dev_open = xspi_nor_dev_open,
};

static const io_dev_funcs_t xspi_nor_funcs = {
	.type		= device_type_xspi_nor,
	.open		= xspi_nor_open,
	.seek		= xspi_nor_seek,
	.size		= xspi_nor_size,
	.read		= xspi_nor_read,
	.write		= xspi_nor_write,
	.erase		= xspi_nor_erase,
	.close		= xspi_nor_close,
	.dev_init	= NULL,
	.dev_close	= xspi_nor_dev_close,
};

static io_dev_info_t xspi_nor_dev_info = {
	.funcs = &xspi_nor_funcs,
	.info = (uintptr_t)NULL,
};

static int xspi_nor_dev_open(const uintptr_t dev_spec __unused,
			     io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	*dev_info = &xspi_nor_dev_info;
	return 0;
}

static int xspi_nor_dev_close(io_dev_info_t *dev_info __unused)
{
	return 0;
}

static int xspi_nor_open(io_dev_info_t *dev_info __unused,
			 const uintptr_t spec, io_entity_t *entity)
{
	const io_block_spec_t *region = (io_block_spec_t *)spec;

	assert(region != NULL);
	assert(entity != NULL);

	if (current_state.in_use != 0) {
		WARN("io_xspi_nor: another region is open; close it first\n");
		return -ENOMEM;
	}

	if (region->length == 0U) {
		return -EINVAL;
	}

	current_state.in_use   = 1;
	current_state.base     = (uint32_t)region->offset;
	current_state.length   = (uint32_t)region->length;
	current_state.file_pos = 0U;
	entity->info = (uintptr_t)&current_state;

	return 0;
}

static int xspi_nor_seek(io_entity_t *entity, int mode,
			 signed long long offset)
{
	xspi_nor_state_t *st;

	assert(entity != NULL);
	st = (xspi_nor_state_t *)entity->info;

	if (mode != IO_SEEK_SET) {
		return -ENOTSUP;
	}
	if ((offset < 0) || ((unsigned long long)offset > (unsigned long long)st->length)) {
		return -EINVAL;
	}

	st->file_pos = (uint32_t)offset;
	return 0;
}

static int xspi_nor_size(io_entity_t *entity, size_t *length)
{
	assert(entity != NULL);
	assert(length != NULL);

	*length = (size_t)((xspi_nor_state_t *)entity->info)->length;
	return 0;
}

static int xspi_nor_read(io_entity_t *entity, uintptr_t buffer,
			 size_t length, size_t *length_read)
{
	xspi_nor_state_t *st;
	uint32_t pos_after;
	int ret;

	assert(entity != NULL);
	assert(length_read != NULL);

	st = (xspi_nor_state_t *)entity->info;

	pos_after = st->file_pos + (uint32_t)length;
	if ((pos_after < st->file_pos) || (pos_after > st->length)) {
		return -EINVAL;
	}

	/*
	 * xspi_read() picks AHB- vs IP-mode based on alignment; this
	 * keeps the backend chip-agnostic and avoids reimplementing
	 * the same heuristic here.
	 */
	ret = xspi_read(st->base + st->file_pos, (uint32_t *)buffer,
			(uint32_t)length);
	if (ret != 0) {
		return -EIO;
	}

	st->file_pos = pos_after;
	*length_read = length;
	return 0;
}

static int xspi_nor_write(io_entity_t *entity, const uintptr_t buffer,
			  size_t length, size_t *length_written)
{
	xspi_nor_state_t *st;
	uint32_t pos_after;
	int ret;

	assert(entity != NULL);
	assert(length_written != NULL);

	st = (xspi_nor_state_t *)entity->info;

	pos_after = st->file_pos + (uint32_t)length;
	if ((pos_after < st->file_pos) || (pos_after > st->length)) {
		return -EINVAL;
	}

	ret = xspi_write(st->base + st->file_pos, (void *)buffer,
			 (uint32_t)length);
	if (ret != 0) {
		return -EIO;
	}
	/*
	 * xspi_write() invalidates the controller's AHB read buffer
	 * internally on completion (drivers/nxp/flexspi/nor/fspi.c),
	 * so any subsequent AHB-mapped read sees the freshly
	 * programmed bytes. No additional flush required here.
	 */

	st->file_pos = pos_after;
	*length_written = length;
	return 0;
}

static int xspi_nor_erase(io_entity_t *entity, size_t length)
{
	xspi_nor_state_t *st;
	uint32_t pos_after;
	int ret;

	assert(entity != NULL);
	st = (xspi_nor_state_t *)entity->info;

	pos_after = st->file_pos + (uint32_t)length;
	if ((pos_after < st->file_pos) || (pos_after > st->length)) {
		return -EINVAL;
	}

	/*
	 * The underlying primitive rounds the erase length up to the
	 * driver's sector granularity (F_SECTOR_ERASE_SZ). Callers
	 * that need byte-exact erase must pre-align both file_pos and
	 * length to the sector boundary themselves.
	 */
	ret = xspi_sector_erase(st->base + st->file_pos, (uint32_t)length);
	if (ret != 0) {
		return -EIO;
	}
	/* xspi_sector_erase() invalidates the AHB read buffer internally. */
	return 0;
}

static int xspi_nor_close(io_entity_t *entity)
{
	assert(entity != NULL);

	entity->info = 0;
	zeromem(&current_state, sizeof(current_state));
	return 0;
}

int register_io_dev_xspi_nor(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);

	result = io_register_device(&xspi_nor_dev_info);
	if (result == 0) {
		*dev_con = &xspi_nor_connector;
	}
	return result;
}
