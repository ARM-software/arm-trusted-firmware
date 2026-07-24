/*
 * Copyright (c) 2014-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_storage.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_image_package.h>
#include <tools_share/uuid.h>

#ifndef MAX_FIP_DEVICES
#define MAX_FIP_DEVICES		1
#endif

/* Useful for printing UUIDs when debugging.*/
#define PRINT_UUID2(x)								\
	"%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",	\
		x.time_low, x.time_mid, x.time_hi_and_version,			\
		x.clock_seq_hi_and_reserved, x.clock_seq_low,			\
		x.node[0], x.node[1], x.node[2], x.node[3],			\
		x.node[4], x.node[5]

typedef struct {
	unsigned int file_pos;
	size_t fip_size;
	fip_toc_entry_t entry;
} fip_file_state_t;

/*
 * Maintain dev_spec per FIP Device
 * TODO - Add backend handles and file state
 * per FIP device here once backends like io_memmap
 * can support multiple open files
 */
typedef struct {
	uintptr_t dev_spec;
	size_t fip_size;
	uint16_t plat_toc_flag;
} fip_dev_state_t;

/*
 * Only one file can be open across all FIP device
 * as backends like io_memmap don't support
 * multiple open files. The file state and
 * backend handle should be maintained per FIP device
 * if the same support is available in the backend
 */
static fip_file_state_t current_fip_file = {0};
static uintptr_t backend_dev_handle;
static uintptr_t backend_image_spec;

static fip_dev_state_t state_pool[MAX_FIP_DEVICES];
static io_dev_info_t dev_info_pool[MAX_FIP_DEVICES];

/* Track number of allocated fip devices */
static unsigned int fip_dev_count;

/* Firmware Image Package driver functions */
static int fip_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int fip_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
			  io_entity_t *entity);
static int fip_file_len(io_entity_t *entity, size_t *length);
static int fip_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
			  size_t *length_read);
static int fip_file_close(io_entity_t *entity);
static int fip_dev_init(io_dev_info_t *dev_info, const uintptr_t init_params);
static int fip_dev_close(io_dev_info_t *dev_info);


/* Return 0 for equal uuids. */
static inline int compare_uuids(const uuid_t *uuid1, const uuid_t *uuid2)
{
	return memcmp(uuid1, uuid2, sizeof(uuid_t));
}


static inline int is_valid_header(fip_toc_header_t *header)
{
	if ((header->name == TOC_HEADER_NAME) && (header->serial_number != 0)) {
		return 1;
	} else {
		return 0;
	}
}


/* Identify the device type as a virtual driver */
static io_type_t device_type_fip(void)
{
	return IO_TYPE_FIRMWARE_IMAGE_PACKAGE;
}


static const io_dev_connector_t fip_dev_connector = {
	.dev_open = fip_dev_open
};


static const io_dev_funcs_t fip_dev_funcs = {
	.type = device_type_fip,
	.open = fip_file_open,
	.seek = NULL,
	.size = fip_file_len,
	.read = fip_file_read,
	.write = NULL,
	.close = fip_file_close,
	.dev_init = fip_dev_init,
	.dev_close = fip_dev_close,
};

/* Locate a file state in the pool, specified by address */
static int find_first_fip_state(const uintptr_t dev_spec,
				  unsigned int *index_out)
{
	int result = -ENOENT;
	unsigned int index;

	for (index = 0; index < (unsigned int)MAX_FIP_DEVICES; ++index) {
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

	if (fip_dev_count < (unsigned int)MAX_FIP_DEVICES) {
		unsigned int index = 0;

		result = find_first_fip_state(0, &index);
		assert(result == 0);
		/* initialize dev_info */
		dev_info_pool[index].funcs = &fip_dev_funcs;
		dev_info_pool[index].info =
				(uintptr_t)&state_pool[index];
		*dev_info = &dev_info_pool[index];
		++fip_dev_count;
	}

	return result;
}

/* Release a device info to the pool */
static int free_dev_info(io_dev_info_t *dev_info)
{
	int result;
	unsigned int index = 0;
	fip_dev_state_t *state;

	assert(dev_info != NULL);

	state = (fip_dev_state_t *)dev_info->info;
	result = find_first_fip_state(state->dev_spec, &index);
	if (result ==  0) {
		/* free if device info is valid */
		zeromem(state, sizeof(fip_dev_state_t));
		--fip_dev_count;
	}

	return result;
}

/*
 * Multiple FIP devices can be opened depending on the value of
 * MAX_FIP_DEVICES. Given that there is only one backend, only a
 * single file can be open at a time by any FIP device.
 */
static int fip_dev_open(const uintptr_t dev_spec,
			 io_dev_info_t **dev_info)
{
	int result;
	io_dev_info_t *info;
	fip_dev_state_t *state;

	assert(dev_info != NULL);
#if MAX_FIP_DEVICES > 1
	assert(dev_spec != (uintptr_t)NULL);
#endif

	result = allocate_dev_info(&info);
	if (result != 0)
		return -ENOMEM;

	state = (fip_dev_state_t *)info->info;

	state->dev_spec = dev_spec;

	*dev_info = info;

	return 0;
}


/* Do some basic package checks. */
static int fip_dev_init(io_dev_info_t *dev_info, const uintptr_t init_params)
{
	int result;
	unsigned int image_id = (unsigned int)init_params;
	uintptr_t backend_handle;
	fip_toc_header_t header;
	size_t bytes_read;
	fip_dev_state_t *state;

	assert(dev_info != NULL);

	state = (fip_dev_state_t *)dev_info->info;

	/* Obtain a reference to the image by querying the platform layer */
	result = plat_get_image_source(image_id, &backend_dev_handle,
				       &backend_image_spec);
	if (result != 0) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
			image_id, result);
		result = -ENOENT;
		goto fip_dev_init_exit;
	}

	/* Attempt to access the FIP image */
	result = io_open(backend_dev_handle, backend_image_spec,
			 &backend_handle);
	if (result != 0) {
		WARN("Failed to access image id=%u (%i)\n", image_id, result);
		result = -ENOENT;
		goto fip_dev_init_exit;
	}

	result = io_size(backend_handle, &state->fip_size);
	if (result != 0) {
		ERROR("Failed to determine FIP size (%i)\n", result);
		result = -EIO;
		goto fip_dev_init_close;
	}

	if (state->fip_size < sizeof(fip_toc_header_t)) {
		ERROR("FIP size too small (%zu)\n", state->fip_size);
		result = -EINVAL;
		goto fip_dev_init_close;
	}

	result = io_read(backend_handle, (uintptr_t)&header, sizeof(header),
			&bytes_read);
	if (result == 0) {
		if (bytes_read != sizeof(header)) {
			ERROR("Short read on FIP header\n");
			result = -EIO;
			goto fip_dev_init_close;
		}
		if (!is_valid_header(&header)) {
			WARN("Firmware Image Package header check failed.\n");
			result = -ENOENT;
		} else {
			VERBOSE("FIP header looks OK.\n");
			/*
			 * Store 16-bit Platform ToC flags field which occupies
			 * bits [32-47] in fip header.
			 */
			state->plat_toc_flag = (header.flags >> 32) & 0xffff;
		}
	}

fip_dev_init_close:
	io_close(backend_handle);

 fip_dev_init_exit:
	return result;
}

/* Close a connection to the FIP device */
static int fip_dev_close(io_dev_info_t *dev_info)
{
	/* TODO: Consider tracking open files and cleaning them up here */

	/* Clear the backend. */
	backend_dev_handle = (uintptr_t)NULL;
	backend_image_spec = (uintptr_t)NULL;

	return free_dev_info(dev_info);
}


/* Open a file for access from package. */
static int fip_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
			 io_entity_t *entity)
{
	int result;
	uintptr_t backend_handle;
	const io_uuid_spec_t *uuid_spec = (io_uuid_spec_t *)spec;
	static const uuid_t uuid_null = { {0} }; /* Double braces for clang */
	size_t bytes_read;
	size_t toc_offset;
	size_t fip_size;
	int found_file = 0;
	fip_dev_state_t *state;

	assert(uuid_spec != NULL);
	assert(entity != NULL);
	assert(dev_info != NULL);

	state = (fip_dev_state_t *)dev_info->info;
	fip_size = state->fip_size;
	if (fip_size == 0U) {
		ERROR("FIP size unknown\n");
		return -EINVAL;
	}
	if (fip_size < (sizeof(fip_toc_header_t) + sizeof(fip_toc_entry_t))) {
		ERROR("FIP size too small for ToC\n");
		return -EINVAL;
	}

	/* Can only have one file open at a time for the moment. We need to
	 * track state like file cursor position. We know the header lives at
	 * offset zero, so this entry should never be zero for an active file.
	 * When the system supports dynamic memory allocation we can allow more
	 * than one open file at a time if needed.
	 */
	if (current_fip_file.entry.offset_address != 0U) {
		WARN("fip_file_open : Only one open file at a time.\n");
		return -ENFILE;
	}

	/* Attempt to access the FIP image */
	result = io_open(backend_dev_handle, backend_image_spec,
			 &backend_handle);
	if (result != 0) {
		WARN("Failed to open Firmware Image Package (%i)\n", result);
		result = -ENOENT;
		goto fip_file_open_exit;
	}

	/* Seek past the FIP header into the Table of Contents */
	result = io_seek(backend_handle, IO_SEEK_SET,
			 (signed long long)sizeof(fip_toc_header_t));
	if (result != 0) {
		WARN("fip_file_open: failed to seek\n");
		result = -ENOENT;
		goto fip_file_open_close;
	}

	found_file = 0;
	toc_offset = sizeof(fip_toc_header_t);
	do {
		if (toc_offset > (fip_size - sizeof(current_fip_file.entry))) {
			ERROR("FIP ToC out of bounds\n");
			result = -EINVAL;
			goto fip_file_open_close;
		}

		result = io_read(backend_handle,
				 (uintptr_t)&current_fip_file.entry,
				 sizeof(current_fip_file.entry),
				 &bytes_read);
		if (result == 0) {
			if (bytes_read != sizeof(current_fip_file.entry)) {
				ERROR("Short read on FIP ToC entry\n");
				result = -EIO;
				goto fip_file_open_close;
			}
			if (compare_uuids(&current_fip_file.entry.uuid,
					  &uuid_spec->uuid) == 0) {
				found_file = 1;
			}
		} else {
			WARN("Failed to read FIP (%i)\n", result);
			goto fip_file_open_close;
		}

		toc_offset += sizeof(current_fip_file.entry);
	} while ((found_file == 0) &&
			(compare_uuids(&current_fip_file.entry.uuid,
				&uuid_null) != 0));

	if (found_file == 1) {
		uint64_t offset;
		uint64_t size;
		uint64_t fip_size64;

		offset = current_fip_file.entry.offset_address;
		size = current_fip_file.entry.size;
		fip_size64 = (uint64_t)fip_size;

		if ((size == 0U) ||
		    (offset >= fip_size64) ||
		    (size > fip_size64) ||
		    (offset + size < offset) ||
		    (offset + size > fip_size64) ||
		    (offset > (uint64_t)SIZE_MAX) ||
		    (size > (uint64_t)SIZE_MAX) ||
		    (offset + size > (uint64_t)SIZE_MAX)) {
			ERROR("FIP entry bounds invalid\n");
			result = -EINVAL;
			goto fip_file_open_close;
		}

		/* All fine. Update entity info with file state and return. Set
		 * the file position to 0. The 'current_fip_file.entry' holds
		 * the base and size of the file.
		 */
		current_fip_file.file_pos = 0;
		current_fip_file.fip_size = fip_size;
		entity->info = (uintptr_t)&current_fip_file;
	} else {
		/* Did not find the file in the FIP. */
		current_fip_file.entry.offset_address = 0;
		result = -ENOENT;
	}

 fip_file_open_close:
	io_close(backend_handle);

 fip_file_open_exit:
	if (result != 0) {
		zeromem(&current_fip_file, sizeof(current_fip_file));
	}
	return result;
}


/* Return the size of a file in package */
static int fip_file_len(io_entity_t *entity, size_t *length)
{
	assert(entity != NULL);
	assert(length != NULL);

	*length =  ((fip_file_state_t *)entity->info)->entry.size;

	return 0;
}


/* Read data from a file in package */
static int fip_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
			  size_t *length_read)
{
	int result;
	fip_file_state_t *fp;
	size_t bytes_read;
	uintptr_t backend_handle;
	size_t remaining;
	uint64_t fip_size64;
	uint64_t file_offset64;

	assert(entity != NULL);
	assert(length_read != NULL);
	assert(entity->info != (uintptr_t)NULL);

	/* Open the backend, attempt to access the blob image */
	result = io_open(backend_dev_handle, backend_image_spec,
			 &backend_handle);
	if (result != 0) {
		WARN("Failed to open FIP (%i)\n", result);
		result = -ENOENT;
		goto fip_file_read_exit;
	}

	fp = (fip_file_state_t *)entity->info;

	if (fp->file_pos > fp->entry.size) {
		ERROR("FIP read: file position out of bounds\n");
		result = -EINVAL;
		goto fip_file_read_close;
	}

	remaining = (size_t)(fp->entry.size - fp->file_pos);
	if (length > remaining) {
		ERROR("FIP read: length out of bounds\n");
		result = -EINVAL;
		goto fip_file_read_close;
	}

	fip_size64 = (uint64_t)fp->fip_size;
	file_offset64 = fp->entry.offset_address + fp->file_pos;
	if ((file_offset64 < fp->entry.offset_address) ||
	    (file_offset64 + length < file_offset64) ||
	    (file_offset64 + length > fip_size64) ||
	    (file_offset64 > (uint64_t)LLONG_MAX)) {
		ERROR("FIP read: offset out of bounds\n");
		result = -EINVAL;
		goto fip_file_read_close;
	}

	/* Seek to the position in the FIP where the payload lives */
	result = io_seek(backend_handle, IO_SEEK_SET,
			 (signed long long)file_offset64);
	if (result != 0) {
		WARN("fip_file_read: failed to seek\n");
		result = -ENOENT;
		goto fip_file_read_close;
	}

	result = io_read(backend_handle, buffer, length, &bytes_read);
	if (result != 0) {
		/* We cannot read our data. Fail. */
		WARN("Failed to read payload (%i)\n", result);
		result = -ENOENT;
		goto fip_file_read_close;
	} else if (bytes_read != length) {
		ERROR("Short read on FIP payload\n");
		result = -EIO;
		goto fip_file_read_close;
	} else {
		/* Set caller length and new file position. */
		*length_read = bytes_read;
		fp->file_pos += bytes_read;
	}

/* Close the backend. */
 fip_file_read_close:
	io_close(backend_handle);

 fip_file_read_exit:
	return result;
}


/* Close a file in package */
static int fip_file_close(io_entity_t *entity)
{
	/* Clear our current file pointer.
	 * If we had malloc() we would free() here.
	 */
	if (current_fip_file.entry.offset_address != 0U) {
		zeromem(&current_fip_file, sizeof(current_fip_file));
	}

	/* Clear the Entity info. */
	entity->info = 0;

	return 0;
}

/* Exported functions */

/* Register the Firmware Image Package driver with the IO abstraction */
int register_io_dev_fip(const io_dev_connector_t **dev_con)
{
	int result;
	assert(dev_con != NULL);

	/*
	 * Since dev_info isn't really used in io_register_device, always
	 * use the same device info at here instead.
	 */
	result = io_register_device(&dev_info_pool[0]);
	if (result == 0)
		*dev_con = &fip_dev_connector;

	return result;
}

/* Function to retrieve plat_toc_flags, previously saved in FIP dev */
int fip_dev_get_plat_toc_flag(io_dev_info_t *dev_info, uint16_t *plat_toc_flag)
{
	fip_dev_state_t *state;

	assert(dev_info != NULL);

	state = (fip_dev_state_t *)dev_info->info;

	*plat_toc_flag =  state->plat_toc_flag;

	return 0;
}
