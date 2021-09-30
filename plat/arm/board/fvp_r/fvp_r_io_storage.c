/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_semihosting.h>
#include <drivers/io/io_storage.h>
#include <lib/semihosting.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/common_def.h>

/* Semihosting filenames */
#define BL33_IMAGE_NAME			"bl33.bin"

#if TRUSTED_BOARD_BOOT
#define TRUSTED_KEY_CERT_NAME		"trusted_key.crt"
#define NT_FW_KEY_CERT_NAME		"nt_fw_key.crt"
#define NT_FW_CONTENT_CERT_NAME		"nt_fw_content.crt"
#endif /* TRUSTED_BOARD_BOOT */

/* IO devices */
static const io_dev_connector_t *sh_dev_con;
static uintptr_t sh_dev_handle;

static const io_file_spec_t sh_file_spec[] = {
	[BL33_IMAGE_ID] = {
		.path = BL33_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_KEY_CERT_ID] = {
		.path = TRUSTED_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		.path = NT_FW_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		.path = NT_FW_CONTENT_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
#endif /* TRUSTED_BOARD_BOOT */
};


static int open_semihosting(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if the file exists on semi-hosting.*/
	result = io_dev_init(sh_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(sh_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using Semi-hosting IO\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

void plat_arm_io_setup(void)
{
	int io_result;

	io_result = arm_io_setup();
	if (io_result < 0) {
		panic();
	}

	/* Register the additional IO devices on this platform */
	io_result = register_io_dev_sh(&sh_dev_con);
	if (io_result < 0) {
		panic();
	}

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(sh_dev_con, (uintptr_t)NULL, &sh_dev_handle);
	if (io_result < 0) {
		panic();
	}
}

/*
 * FVP_R provides semihosting as an alternative to load images
 */
int plat_arm_get_alt_image_source(unsigned int image_id, uintptr_t *dev_handle,
				  uintptr_t *image_spec)
{
	int result = open_semihosting((const uintptr_t)&sh_file_spec[image_id]);

	if (result == 0) {
		*dev_handle = sh_dev_handle;
		*image_spec = (uintptr_t)&sh_file_spec[image_id];
	}

	return result;
}
