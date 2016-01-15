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
#include <io_driver.h>
#include <io_storage.h>
#include <semihosting.h>



/* Identify the device type as semihosting */
static io_type_t device_type_sh(void)
{
	return IO_TYPE_SEMIHOSTING;
}


/* Semi-hosting functions, device info and handle */

static int sh_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int sh_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
		io_entity_t *entity);
static int sh_file_seek(io_entity_t *entity, int mode, ssize_t offset);
static int sh_file_len(io_entity_t *entity, size_t *length);
static int sh_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		size_t *length_read);
static int sh_file_write(io_entity_t *entity, const uintptr_t buffer,
		size_t length, size_t *length_written);
static int sh_file_close(io_entity_t *entity);

static const io_dev_connector_t sh_dev_connector = {
	.dev_open = sh_dev_open
};


static const io_dev_funcs_t sh_dev_funcs = {
	.type = device_type_sh,
	.open = sh_file_open,
	.seek = sh_file_seek,
	.size = sh_file_len,
	.read = sh_file_read,
	.write = sh_file_write,
	.close = sh_file_close,
	.dev_init = NULL,	/* NOP */
	.dev_close = NULL,	/* NOP */
};


/* No state associated with this device so structure can be const */
static const io_dev_info_t sh_dev_info = {
	.funcs = &sh_dev_funcs,
	.info = (uintptr_t)NULL
};


/* Open a connection to the semi-hosting device */
static int sh_dev_open(const uintptr_t dev_spec __unused,
		io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&sh_dev_info; /* cast away const */
	return 0;
}


/* Open a file on the semi-hosting device */
static int sh_file_open(io_dev_info_t *dev_info __unused,
		const uintptr_t spec, io_entity_t *entity)
{
	int result = -ENOENT;
	long sh_result = -1;
	const io_file_spec_t *file_spec = (const io_file_spec_t *)spec;

	assert(file_spec != NULL);
	assert(entity != NULL);

	sh_result = semihosting_file_open(file_spec->path, file_spec->mode);

	if (sh_result > 0) {
		entity->info = (uintptr_t)sh_result;
		result = 0;
	}
	return result;
}


/* Seek to a particular file offset on the semi-hosting device */
static int sh_file_seek(io_entity_t *entity, int mode, ssize_t offset)
{
	long file_handle, sh_result;

	assert(entity != NULL);

	file_handle = (long)entity->info;

	sh_result = semihosting_file_seek(file_handle, offset);

	return (sh_result == 0) ? 0 : -ENOENT;
}


/* Return the size of a file on the semi-hosting device */
static int sh_file_len(io_entity_t *entity, size_t *length)
{
	int result = -ENOENT;

	assert(entity != NULL);
	assert(length != NULL);

	long sh_handle = (long)entity->info;
	long sh_result = semihosting_file_length(sh_handle);

	if (sh_result >= 0) {
		result = 0;
		*length = (size_t)sh_result;
	}

	return result;
}


/* Read data from a file on the semi-hosting device */
static int sh_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
		size_t *length_read)
{
	int result = -ENOENT;
	long sh_result = -1;
	size_t bytes = length;
	long file_handle;

	assert(entity != NULL);
	assert(buffer != (uintptr_t)NULL);
	assert(length_read != NULL);

	file_handle = (long)entity->info;

	sh_result = semihosting_file_read(file_handle, &bytes, buffer);

	if (sh_result >= 0) {
		*length_read = (bytes != length) ? bytes : length;
		result = 0;
	}

	return result;
}


/* Write data to a file on the semi-hosting device */
static int sh_file_write(io_entity_t *entity, const uintptr_t buffer,
		size_t length, size_t *length_written)
{
	long sh_result = -1;
	long file_handle;
	size_t bytes = length;

	assert(entity != NULL);
	assert(buffer != (uintptr_t)NULL);
	assert(length_written != NULL);

	file_handle = (long)entity->info;

	sh_result = semihosting_file_write(file_handle, &bytes, buffer);

	*length_written = length - bytes;

	return (sh_result == 0) ? 0 : -ENOENT;
}


/* Close a file on the semi-hosting device */
static int sh_file_close(io_entity_t *entity)
{
	long sh_result = -1;
	long file_handle;

	assert(entity != NULL);

	file_handle = (long)entity->info;

	sh_result = semihosting_file_close(file_handle);

	return (sh_result >= 0) ? 0 : -ENOENT;
}


/* Exported functions */

/* Register the semi-hosting driver with the IO abstraction */
int register_io_dev_sh(const io_dev_connector_t **dev_con)
{
	int result;
	assert(dev_con != NULL);

	result = io_register_device(&sh_dev_info);
	if (result == 0)
		*dev_con = &sh_dev_connector;

	return result;
}
