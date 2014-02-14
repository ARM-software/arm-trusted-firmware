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
#include "io_fip.h"
#include "io_memmap.h"
#include "debug.h"


typedef struct {
	char *image_name;
	int (*image_policy)(io_dev_handle *dev_handle, void **image_spec);
} plat_io_policy;


/* IO devices */
static struct io_plat_data io_data;
static struct io_dev_connector *sh_dev_con;
static void *const sh_dev_spec;
static void *const sh_init_params;
static io_dev_handle sh_dev_handle;
static struct io_dev_connector *fip_dev_con;
static void *const fip_dev_spec;
static io_dev_handle fip_dev_handle;
static struct io_dev_connector *memmap_dev_con;
static void *const memmap_dev_spec;
static void *const memmap_init_params;
static io_dev_handle memmap_dev_handle;

static int fvp_bl2_policy(io_dev_handle *dev_handle, void **image_spec);
static int fvp_bl31_policy(io_dev_handle *dev_handle, void **image_spec);
static int fvp_bl33_policy(io_dev_handle *dev_handle, void **image_spec);
static int fvp_fip_policy(io_dev_handle *dev_handle, void **image_spec);


static io_block_spec fip_block_spec = {
	.offset = FLASH0_BASE,
	.length = FLASH0_SIZE
};

static io_file_spec bl2_file_spec = {
	.path = BL2_IMAGE_NAME,
	.mode = FOPEN_MODE_R
};

static io_file_spec bl31_file_spec = {
	.path = BL31_IMAGE_NAME,
	.mode = FOPEN_MODE_R
};

static io_file_spec bl33_file_spec = {
	.path = BL33_IMAGE_NAME,
	.mode = FOPEN_MODE_R
};

static plat_io_policy fvp_policy[] = {
	{BL2_IMAGE_NAME,  fvp_bl2_policy},
	{BL31_IMAGE_NAME, fvp_bl31_policy},
	{BL33_IMAGE_NAME, fvp_bl33_policy},
	{FIP_IMAGE_NAME,  fvp_fip_policy},
	{NULL, NULL}
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


static int open_semihosting(void *spec)
{
	int result = IO_FAIL;
	io_handle local_image_handle;

	/* See if the file exists on semi-hosting.*/
	result = io_dev_init(sh_dev_handle, sh_init_params);
	if (result == IO_SUCCESS) {
		result = io_open(sh_dev_handle, spec, &local_image_handle);
		if (result == IO_SUCCESS) {
			INFO("Using Semi-hosting IO\n");
			io_close(local_image_handle);
		}
	}
	return result;
}


/* Try to load BL2 from Firmware Image Package in FLASH first. If there is no
 * FIP in FLASH or it is broken, try to load the file from semi-hosting.
 */
static int fvp_bl2_policy(io_dev_handle *dev_handle, void **image_spec)
{
	int result = IO_FAIL;
	void *local_image_spec = &bl2_file_spec;

	INFO("Loading BL2\n");
	/* FIP first then fall back to semi-hosting */
	result = open_fip(local_image_spec);
	if (result == IO_SUCCESS) {
		*dev_handle = fip_dev_handle;
		*(io_file_spec **)image_spec = local_image_spec;
	} else {
		result = open_semihosting(local_image_spec);
		if (result == IO_SUCCESS) {
			*dev_handle = sh_dev_handle;
			*(io_file_spec **)image_spec = local_image_spec;
		}
	}
	return result;
}


/* Try to load BL31 from Firmware Image Package in FLASH first. If there is no
 * FIP in FLASH or it is broken, try to load the file from semi-hosting.
 */
static int fvp_bl31_policy(io_dev_handle *dev_handle, void **image_spec)
{
	int result = IO_FAIL;
	void *local_image_spec = &bl31_file_spec;

	INFO("Loading BL31\n");
	/* FIP first then fall back to semi-hosting */
	result = open_fip(local_image_spec);
	if (result == IO_SUCCESS) {
		*dev_handle = fip_dev_handle;
		*(io_file_spec **)image_spec = local_image_spec;
	} else {
		result = open_semihosting(local_image_spec);
		if (result == IO_SUCCESS) {
			*dev_handle = sh_dev_handle;
			*(io_file_spec **)image_spec = local_image_spec;
		}
	}
	return result;
}


/* Try to load BL33 from Firmware Image Package in FLASH first. If there is no
 * FIP in FLASH or it is broken, try to load the file from semi-hosting.
 */
static int fvp_bl33_policy(io_dev_handle *dev_handle, void **image_spec)
{
	int result = IO_FAIL;
	void *local_image_spec = &bl33_file_spec;

	INFO("Loading BL33 (UEFI)\n");
	/* FIP first then fall back to semi-hosting */
	result = open_fip(local_image_spec);
	if (result == IO_SUCCESS) {
		*dev_handle = fip_dev_handle;
		*(io_file_spec **)image_spec = local_image_spec;
	} else {
		result = open_semihosting(local_image_spec);
		if (result == IO_SUCCESS) {
			*dev_handle = sh_dev_handle;
			*(io_file_spec **)image_spec = local_image_spec;
		}
	}
	return result;
}


/* Try to find FIP on NOR FLASH */
static int fvp_fip_policy(io_dev_handle *dev_handle, void **image_spec)
{
	int result = IO_FAIL;
	void *local_image_spec = &fip_block_spec;

	result = open_memmap(local_image_spec);
	if (result == IO_SUCCESS) {
		*dev_handle = memmap_dev_handle;
		*(io_file_spec **)image_spec = local_image_spec;
	}
	return result;
}


void io_setup (void)
{
	int io_result = IO_FAIL;

	/* Initialise the IO layer */
	io_init(&io_data);

	/* Register the IO devices on this platform */
	io_result = register_io_dev_sh(&sh_dev_con);
	assert(io_result == IO_SUCCESS);

	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == IO_SUCCESS);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == IO_SUCCESS);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(sh_dev_con, sh_dev_spec, &sh_dev_handle);
	assert(io_result == IO_SUCCESS);

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

	assert(image_name != NULL);
	assert(dev_handle != NULL);
	assert(image_spec != NULL);

	policy = fvp_policy;
	while ((policy->image_name != NULL) &&
	       (policy->image_policy != NULL)) {
		result = strcmp(policy->image_name, image_name);
		if (result == 0) {
			result = policy->image_policy(dev_handle, image_spec);
			break;
		}
		policy++;
	}
	return result;
}
