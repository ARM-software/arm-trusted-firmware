/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <firmware_image_package.h>
#include <io_driver.h>
#include <io_fip.h>
#include <io_storage.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <string.h>
#include <uuid.h>

/* Useful for printing UUIDs when debugging.*/
#define PRINT_UUID2(x)								\
	"%08x-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",	\
		x.time_low, x.time_mid, x.time_hi_and_version,			\
		x.clock_seq_hi_and_reserved, x.clock_seq_low,			\
		x.node[0], x.node[1], x.node[2], x.node[3],			\
		x.node[4], x.node[5]

typedef struct {
	/* Put file_pos above the struct to allow {0} on static init.
	 * It is a workaround for a known bug in GCC
	 * http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
	 */
	unsigned int file_pos;
	fip_toc_entry_t entry;
} file_state_t;

static const uuid_t uuid_null = {0};
static file_state_t current_file = {0};
static uintptr_t backend_dev_handle;
static uintptr_t backend_image_spec;


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


/* TODO: We could check version numbers or do a package checksum? */
static inline int is_valid_header(fip_toc_header_t *header)
{
	if ((header->name == TOC_HEADER_NAME) && (header->serial_number != 0)) {
		return 1;
	} else {
		return 0;
	}
}


/* Identify the device type as a virtual driver */
io_type_t device_type_fip(void)
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


/* No state associated with this device so structure can be const */
static const io_dev_info_t fip_dev_info = {
	.funcs = &fip_dev_funcs,
	.info = (uintptr_t)NULL
};


/* Open a connection to the FIP device */
static int fip_dev_open(const uintptr_t dev_spec __unused,
			 io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&fip_dev_info; /* cast away const */

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

	result = io_read(backend_handle, (uintptr_t)&header, sizeof(header),
			&bytes_read);
	if (result == 0) {
		if (!is_valid_header(&header)) {
			WARN("Firmware Image Package header check failed.\n");
			result = -ENOENT;
		} else {
			VERBOSE("FIP header looks OK.\n");
		}
	}

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

	return 0;
}


/* Open a file for access from package. */
static int fip_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
			 io_entity_t *entity)
{
	int result;
	uintptr_t backend_handle;
	const io_uuid_spec_t *uuid_spec = (io_uuid_spec_t *)spec;
	size_t bytes_read;
	int found_file = 0;

	assert(uuid_spec != NULL);
	assert(entity != NULL);

	/* Can only have one file open at a time for the moment. We need to
	 * track state like file cursor position. We know the header lives at
	 * offset zero, so this entry should never be zero for an active file.
	 * When the system supports dynamic memory allocation we can allow more
	 * than one open file at a time if needed.
	 */
	if (current_file.entry.offset_address != 0) {
		WARN("fip_file_open : Only one open file at a time.\n");
		return -ENOMEM;
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
	result = io_seek(backend_handle, IO_SEEK_SET, sizeof(fip_toc_header_t));
	if (result != 0) {
		WARN("fip_file_open: failed to seek\n");
		result = -ENOENT;
		goto fip_file_open_close;
	}

	found_file = 0;
	do {
		result = io_read(backend_handle,
				 (uintptr_t)&current_file.entry,
				 sizeof(current_file.entry),
				 &bytes_read);
		if (result == 0) {
			if (compare_uuids(&current_file.entry.uuid,
					  &uuid_spec->uuid) == 0) {
				found_file = 1;
				break;
			}
		} else {
			WARN("Failed to read FIP (%i)\n", result);
			goto fip_file_open_close;
		}
	} while (compare_uuids(&current_file.entry.uuid, &uuid_null) != 0);

	if (found_file == 1) {
		/* All fine. Update entity info with file state and return. Set
		 * the file position to 0. The 'current_file.entry' holds the
		 * base and size of the file.
		 */
		current_file.file_pos = 0;
		entity->info = (uintptr_t)&current_file;
	} else {
		/* Did not find the file in the FIP. */
		current_file.entry.offset_address = 0;
		result = -ENOENT;
	}

 fip_file_open_close:
	io_close(backend_handle);

 fip_file_open_exit:
	return result;
}


/* Return the size of a file in package */
static int fip_file_len(io_entity_t *entity, size_t *length)
{
	assert(entity != NULL);
	assert(length != NULL);

	*length =  ((file_state_t *)entity->info)->entry.size;

	return 0;
}


/* Read data from a file in package */
static int fip_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
			  size_t *length_read)
{
	int result;
	file_state_t *fp;
	size_t file_offset;
	size_t bytes_read;
	uintptr_t backend_handle;

	assert(entity != NULL);
	assert(buffer != (uintptr_t)NULL);
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

	fp = (file_state_t *)entity->info;

	/* Seek to the position in the FIP where the payload lives */
	file_offset = fp->entry.offset_address + fp->file_pos;
	result = io_seek(backend_handle, IO_SEEK_SET, file_offset);
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
	if (current_file.entry.offset_address != 0) {
		memset(&current_file, 0, sizeof(current_file));
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

	result = io_register_device(&fip_dev_info);
	if (result == 0)
		*dev_con = &fip_dev_connector;

	return result;
}
