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
#include "io_semihosting.h"
#include "semihosting.h"	/* For FOPEN_MODE_... */
#include "debug.h"


/* IO devices */
static struct io_plat_data io_data;
static struct io_dev_connector *sh_dev_con;
static void *const sh_dev_spec;
static void *const sh_init_params;
static io_dev_handle sh_dev_handle;

static io_file_spec bl2_image_spec = {
	.path = BL2_IMAGE_NAME,
	.mode = FOPEN_MODE_R
};

static io_file_spec bl31_image_spec = {
	.path = BL31_IMAGE_NAME,
	.mode = FOPEN_MODE_R
};


/* Set up the IO devices present on this platform, ready for use */
void io_setup(void)
{
	/* Initialise the IO layer */
	io_init(&io_data);

	/* Register a semi-hosting device */
	int io_result = register_io_dev_sh(&sh_dev_con);
	assert(io_result == IO_SUCCESS);

	/* Open a connection to the semi-hosting device and cache the handle */
	io_result = io_dev_open(sh_dev_con, sh_dev_spec, &sh_dev_handle);
	assert(io_result == IO_SUCCESS);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}


/* Return an IO device handle and specification which can be used to access
 * an image */
int plat_get_image_source(const char *image_name, io_dev_handle *dev_handle,
				void **image_spec)
{
	int result = IO_FAIL;
	assert((image_name != NULL) && (dev_handle != NULL) &&
			(image_spec != NULL));

	if (strcmp(BL2_IMAGE_NAME, image_name) == 0) {
		result = io_dev_init(sh_dev_handle, sh_init_params);
		if (result == IO_SUCCESS) {
			*dev_handle = sh_dev_handle;
			*(io_file_spec **)image_spec = &bl2_image_spec;
		}
	} else if (strcmp(BL31_IMAGE_NAME, image_name) == 0) {
		result = io_dev_init(sh_dev_handle, sh_init_params);
		if (result == IO_SUCCESS) {
			*dev_handle = sh_dev_handle;
			*(io_file_spec **)image_spec = &bl31_image_spec;
		}
	} else
		assert(0);

	return result;
}
