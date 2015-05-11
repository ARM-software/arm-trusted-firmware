/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <io_fip.h>
#include <io_memmap.h>
#include <io_storage.h>
#include <platform_def.h>
#include <semihosting.h>	/* For FOPEN_MODE_... */
#include <string.h>

/* IO devices */
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static const io_block_spec_t fip_block_spec = {
	.offset = PLAT_ARM_FIP_BASE,
	.length = PLAT_ARM_FIP_MAX_SIZE
};

static const io_file_spec_t bl2_file_spec = {
	.path = BL2_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl30_file_spec = {
	.path = BL30_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl31_file_spec = {
	.path = BL31_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl32_file_spec = {
	.path = BL32_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl33_file_spec = {
	.path = BL33_IMAGE_NAME,
	.mode = FOPEN_MODE_RB
};

#if TRUSTED_BOARD_BOOT
static const io_file_spec_t bl2_cert_file_spec = {
	.path = BL2_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t trusted_key_cert_file_spec = {
	.path = TRUSTED_KEY_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl30_key_cert_file_spec = {
	.path = BL30_KEY_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl31_key_cert_file_spec = {
	.path = BL31_KEY_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl32_key_cert_file_spec = {
	.path = BL32_KEY_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl33_key_cert_file_spec = {
	.path = BL33_KEY_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl30_cert_file_spec = {
	.path = BL30_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl31_cert_file_spec = {
	.path = BL31_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl32_cert_file_spec = {
	.path = BL32_CERT_NAME,
	.mode = FOPEN_MODE_RB
};

static const io_file_spec_t bl33_cert_file_spec = {
	.path = BL33_CERT_NAME,
	.mode = FOPEN_MODE_RB
};
#endif /* TRUSTED_BOARD_BOOT */

static int open_fip(const uintptr_t spec);
static int open_memmap(const uintptr_t spec);

struct plat_io_policy {
	const char *image_name;
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	{
		FIP_IMAGE_NAME,
		&memmap_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_memmap
	}, {
		BL2_IMAGE_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl2_file_spec,
		open_fip
	}, {
		BL30_IMAGE_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl30_file_spec,
		open_fip
	}, {
		BL31_IMAGE_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl31_file_spec,
		open_fip
	}, {
		BL32_IMAGE_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl32_file_spec,
		open_fip
	}, {
		BL33_IMAGE_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl33_file_spec,
		open_fip
	}, {
#if TRUSTED_BOARD_BOOT
		BL2_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl2_cert_file_spec,
		open_fip
	}, {
		TRUSTED_KEY_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&trusted_key_cert_file_spec,
		open_fip
	}, {
		BL30_KEY_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl30_key_cert_file_spec,
		open_fip
	}, {
		BL31_KEY_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl31_key_cert_file_spec,
		open_fip
	}, {
		BL32_KEY_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl32_key_cert_file_spec,
		open_fip
	}, {
		BL33_KEY_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl33_key_cert_file_spec,
		open_fip
	}, {
		BL30_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl30_cert_file_spec,
		open_fip
	}, {
		BL31_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl31_cert_file_spec,
		open_fip
	}, {
		BL32_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl32_cert_file_spec,
		open_fip
	}, {
		BL33_CERT_NAME,
		&fip_dev_handle,
		(uintptr_t)&bl33_cert_file_spec,
		open_fip
	}, {
#endif /* TRUSTED_BOARD_BOOT */
		0, 0, 0
	}
};


/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_arm_io_setup
#pragma weak plat_arm_get_alt_image_source


static int open_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_NAME);
	if (result == IO_SUCCESS) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == IO_SUCCESS) {
			VERBOSE("Using FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}


static int open_memmap(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(memmap_dev_handle, (uintptr_t)NULL);
	if (result == IO_SUCCESS) {
		result = io_open(memmap_dev_handle, spec, &local_image_handle);
		if (result == IO_SUCCESS) {
			VERBOSE("Using Memmap\n");
			io_close(local_image_handle);
		}
	}
	return result;
}


void arm_io_setup(void)
{
	int io_result;

	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == IO_SUCCESS);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == IO_SUCCESS);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	assert(io_result == IO_SUCCESS);

	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&memmap_dev_handle);
	assert(io_result == IO_SUCCESS);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}

void plat_arm_io_setup(void)
{
	arm_io_setup();
}

int plat_arm_get_alt_image_source(
	const uintptr_t image_spec __attribute__((unused)),
	uintptr_t *dev_handle __attribute__((unused)))
{
	/* By default do not try an alternative */
	return IO_FAIL;
}

/* Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy */
int plat_get_image_source(const char *image_name, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result = IO_FAIL;
	const struct plat_io_policy *policy;

	if ((image_name != NULL) && (dev_handle != NULL) &&
	    (image_spec != NULL)) {
		policy = policies;
		while (policy->image_name != NULL) {
			if (strcmp(policy->image_name, image_name) == 0) {
				result = policy->check(policy->image_spec);
				if (result == IO_SUCCESS) {
					*image_spec = policy->image_spec;
					*dev_handle = *(policy->dev_handle);
					break;
				}
				VERBOSE("Trying alternative IO\n");
				result = plat_arm_get_alt_image_source(
						policy->image_spec,
						dev_handle);
				if (result == IO_SUCCESS) {
					*image_spec = policy->image_spec;
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
