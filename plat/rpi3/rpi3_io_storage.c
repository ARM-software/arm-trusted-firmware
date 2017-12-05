/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <firmware_image_package.h>
#include <io_driver.h>
#include <io_fip.h>
#include <io_memmap.h>
#include <platform_def.h>
#include <string.h>

/* Semihosting filenames */
#define BL2_IMAGE_NAME			"bl2.bin"
#define BL31_IMAGE_NAME			"bl31.bin"
#define BL32_IMAGE_NAME			"bl32.bin"
#define BL33_IMAGE_NAME			"bl33.bin"

#if TRUSTED_BOARD_BOOT
#define BL2_CERT_NAME			"bl2.crt"
#define TRUSTED_KEY_CERT_NAME		"trusted_key.crt"
#define BL31_KEY_CERT_NAME		"bl31_key.crt"
#define BL32_KEY_CERT_NAME		"bl32_key.crt"
#define BL33_KEY_CERT_NAME		"bl33_key.crt"
#define BL31_CERT_NAME			"bl31.crt"
#define BL32_CERT_NAME			"bl32.crt"
#define BL33_CERT_NAME			"bl33.crt"
#endif /* TRUSTED_BOARD_BOOT */

/* IO devices */
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static const io_block_spec_t fip_block_spec = {
	.offset = PLAT_RPI3_FIP_BASE,
	.length = PLAT_RPI3_FIP_MAX_SIZE
};

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t bl31_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl32_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t bl2_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2_CERT,
};

static const io_uuid_spec_t trusted_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t bl31_key_cert_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31_KEY_CERT,
};

static const io_uuid_spec_t bl32_key_cert_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_KEY_CERT,
};

static const io_uuid_spec_t bl33_key_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33_KEY_CERT,
};

static const io_uuid_spec_t bl31_cert_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31_CERT,
};

static const io_uuid_spec_t bl32_cert_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_CERT,
};

static const io_uuid_spec_t bl33_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33_CERT,
};
#endif /* TRUSTED_BOARD_BOOT */

static int open_fip(const uintptr_t spec);
static int open_memmap(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, load images from the FIP */
static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_memmap
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		open_fip
	},
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		open_fip
	},
	[BL32_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		open_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		open_fip
	},
#if TRUSTED_BOARD_BOOT
	[BL2_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&trusted_key_cert_uuid_spec,
		open_fip
	},
	[BL31_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_key_cert_uuid_spec,
		open_fip
	},
	[BL32_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_key_cert_uuid_spec,
		open_fip
	},
	[BL33_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_key_cert_uuid_spec,
		open_fip
	},
	[BL31_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_cert_uuid_spec,
		open_fip
	},
	[BL32_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_cert_uuid_spec,
		open_fip
	},
	[BL33_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_cert_uuid_spec,
		open_fip
	},
#endif /* TRUSTED_BOARD_BOOT */
};

static int open_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0) {
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
	if (result == 0) {
		result = io_open(memmap_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using Memmap\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

void plat_rpi3_io_setup(void)
{
	int io_result;

	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == 0);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	assert(io_result == 0);

	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&memmap_dev_handle);
	assert(io_result == 0);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	result = policy->check(policy->image_spec);
	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}

	return result;
}
