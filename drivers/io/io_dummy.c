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
#include <io_driver.h>
#include <io_storage.h>
#include <string.h>

struct file_state {
	int in_use;
	size_t size;
};

static struct file_state current_file = {0};

/* Identify the device type as dummy */
io_type_t device_type_dummy(void)
{
	return IO_TYPE_DUMMY;
}

/* Dummy device functions */
static int dummy_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int dummy_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
			     io_entity_t *entity);
static int dummy_block_len(io_entity_t *entity, size_t *length);
static int dummy_block_read(io_entity_t *entity, uintptr_t buffer,
			     size_t length, size_t *length_read);
static int dummy_block_close(io_entity_t *entity);
static int dummy_dev_close(io_dev_info_t *dev_info);


static const io_dev_connector_t dummy_dev_connector = {
	.dev_open = dummy_dev_open
};


static const io_dev_funcs_t dummy_dev_funcs = {
	.type = device_type_dummy,
	.open = dummy_block_open,
	.seek = NULL,
	.size = dummy_block_len,
	.read = dummy_block_read,
	.write = NULL,
	.close = dummy_block_close,
	.dev_init = NULL,
	.dev_close = dummy_dev_close,
};


static const io_dev_info_t dummy_dev_info = {
	.funcs = &dummy_dev_funcs,
	.info = (uintptr_t)NULL
};


/* Open a connection to the dummy device */
static int dummy_dev_open(const uintptr_t dev_spec __attribute__((unused)),
			   io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&dummy_dev_info;

	return 0;
}


/* Close a connection to the dummy device */
static int dummy_dev_close(io_dev_info_t *dev_info)
{
	return 0;
}


/* Open a file on the dummy device */
static int dummy_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
			     io_entity_t *entity)
{
	int result;
	const io_block_spec_t *block_spec = (io_block_spec_t *)spec;

	if (current_file.in_use == 0) {
		assert(block_spec != NULL);
		assert(entity != NULL);

		current_file.in_use = 1;
		current_file.size = block_spec->length;
		entity->info = (uintptr_t)&current_file;
		result = 0;
	} else {
		WARN("A Dummy device is already active. Close first.\n");
		result = -ENOMEM;
	}

	return result;
}


/* Return the size of a file on the dummy device */
static int dummy_block_len(io_entity_t *entity, size_t *length)
{
	assert(entity != NULL);
	assert(length != NULL);

	*length =  ((struct file_state *)entity->info)->size;

	return 0;
}


/* Read data from a file on the dummy device */
static int dummy_block_read(io_entity_t *entity, uintptr_t buffer,
			     size_t length, size_t *length_read)
{
	assert(length_read != NULL);

	*length_read = length;

	return 0;
}


/* Close a file on the dummy device */
static int dummy_block_close(io_entity_t *entity)
{
	assert(entity != NULL);

	entity->info = 0;
	current_file.in_use = 0;

	return 0;
}


/* Exported functions */

/* Register the dummy driver with the IO abstraction */
int register_io_dev_dummy(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);

	result = io_register_device(&dummy_dev_info);
	if (result == 0)
		*dev_con = &dummy_dev_connector;

	return result;
}
