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
#include <string.h>
#include "platform.h"
#include "io_storage.h"
#include "io_driver.h"
#include "semihosting.h"	/* For FOPEN_MODE_... */
#include "io_fip.h"
#include "io_memmap.h"
#include "debug.h"

/* IO devices */
static struct io_plat_data io_data;
static struct io_dev_connector *fip_dev_con;
static void *const fip_dev_spec;
static io_dev_handle fip_dev_handle;
static struct io_dev_connector *memmap_dev_con;
static void *const memmap_dev_spec;
static void *const memmap_init_params;
static io_dev_handle memmap_dev_handle;

static io_block_spec fip_block_spec = {
	.offset = FLASH_BASE,
	.length = FLASH_SIZE
};

static io_file_spec bl2_file_spec = {
	.path = BL2_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

static io_file_spec bl30_file_spec = {
	.path = BL30_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

static io_file_spec bl31_file_spec = {
	.path = BL31_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

static io_file_spec bl33_file_spec = {
	.path = BL33_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

static int open_fip(void *spec);
static int open_memmap(void *spec);

typedef struct {
	char *image_name;
	io_dev_handle *dev_handle;
	void *image_spec;
	int (*check)(void *spec);
} plat_io_policy;

static plat_io_policy policies[] = {
	{ FIP_IMAGE_NAME,  &memmap_dev_handle, &fip_block_spec, open_memmap },
	{ BL2_IMAGE_NAME,  &fip_dev_handle,    &bl2_file_spec,  open_fip    },
	{ BL30_IMAGE_NAME, &fip_dev_handle,    &bl30_file_spec, open_fip    },
	{ BL31_IMAGE_NAME, &fip_dev_handle,    &bl31_file_spec, open_fip    },
	{ BL33_IMAGE_NAME, &fip_dev_handle,    &bl33_file_spec, open_fip    },
	{0, 0, 0}
};


static int open_fip(void *spec)
{
	int result = IO_FAIL;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (void *)FIP_IMAGE_NAME);
	if (result == IO_SUCCESS) {
		INFO("Using FIP\n");
		/*TODO: Check image defined in spec is present in FIP. */
	}
	return result;
}


static int open_memmap(void *spec)
{
	int result = IO_FAIL;
	io_handle local_image_handle;

	result = io_dev_init(memmap_dev_handle, memmap_init_params);
	if (result == IO_SUCCESS) {
		result = io_open(memmap_dev_handle, spec, &local_image_handle);
		if (result == IO_SUCCESS) {
			/* INFO("Using Memmap IO\n"); */
			io_close(local_image_handle);
		}
	}
	return result;
}

void io_setup (void)
{
	int io_result = IO_FAIL;

	/* Initialise the IO layer */
	io_init(&io_data);

	/* Register the IO devices on this platform */
	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == IO_SUCCESS);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == IO_SUCCESS);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, fip_dev_spec, &fip_dev_handle);
	assert(io_result == IO_SUCCESS);

	io_result = io_dev_open(memmap_dev_con, memmap_dev_spec,
				&memmap_dev_handle);
	assert(io_result == IO_SUCCESS);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}


/* Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy */
int plat_get_image_source(const char *image_name, io_dev_handle *dev_handle,
			  void **image_spec)
{
	int result = IO_FAIL;
	plat_io_policy *policy;

	if ((image_name != NULL) && (dev_handle != NULL) &&
	    (image_spec != NULL)) {
		policy = policies;
		while (policy->image_name != NULL) {
			if (strcmp(policy->image_name, image_name) == 0) {
				result = policy->check(policy->image_spec);
				if (result == IO_SUCCESS) {
					*(io_file_spec **)image_spec = policy->image_spec;
					*dev_handle = *(policy->dev_handle);
					break;
				}
			}
			policy++;
		}
	} else {
		result = IO_FAIL;
	}
	return result;
}
