/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_semihosting.h>
#include <drivers/io/io_storage.h>
#include <lib/semihosting.h>
#include <tools_share/firmware_image_package.h>

/* Semihosting filenames */
#define BL2_IMAGE_NAME			"bl2.bin"
#define BL31_IMAGE_NAME			"bl31.bin"
#define BL32_IMAGE_NAME			"bl32.bin"
#define BL32_EXTRA1_IMAGE_NAME		"bl32_extra1.bin"
#define BL32_EXTRA2_IMAGE_NAME		"bl32_extra2.bin"
#define BL33_IMAGE_NAME			"bl33.bin"

#if TRUSTED_BOARD_BOOT
#define TRUSTED_BOOT_FW_CERT_NAME	"tb_fw.crt"
#define TRUSTED_KEY_CERT_NAME		"trusted_key.crt"
#define SOC_FW_KEY_CERT_NAME		"soc_fw_key.crt"
#define TOS_FW_KEY_CERT_NAME		"tos_fw_key.crt"
#define NT_FW_KEY_CERT_NAME		"nt_fw_key.crt"
#define SOC_FW_CONTENT_CERT_NAME	"soc_fw_content.crt"
#define TOS_FW_CONTENT_CERT_NAME	"tos_fw_content.crt"
#define NT_FW_CONTENT_CERT_NAME		"nt_fw_content.crt"
#endif /* TRUSTED_BOARD_BOOT */



/* IO devices */
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;
static const io_dev_connector_t *sh_dev_con;
static uintptr_t sh_dev_handle;

static const io_block_spec_t fip_block_spec = {
	.offset = PLAT_QEMU_FIP_BASE,
	.length = PLAT_QEMU_FIP_MAX_SIZE
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

static const io_uuid_spec_t bl32_extra1_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA1,
};

static const io_uuid_spec_t bl32_extra2_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA2,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t tb_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FW_CERT,
};

static const io_uuid_spec_t trusted_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t soc_fw_key_cert_uuid_spec = {
	.uuid = UUID_SOC_FW_KEY_CERT,
};

static const io_uuid_spec_t tos_fw_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t nt_fw_key_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t soc_fw_cert_uuid_spec = {
	.uuid = UUID_SOC_FW_CONTENT_CERT,
};

static const io_uuid_spec_t tos_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t nt_fw_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif /* TRUSTED_BOARD_BOOT */

static const io_file_spec_t sh_file_spec[] = {
	[BL2_IMAGE_ID] = {
		.path = BL2_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL31_IMAGE_ID] = {
		.path = BL31_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL32_IMAGE_ID] = {
		.path = BL32_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		.path = BL32_EXTRA1_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		.path = BL32_EXTRA2_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL33_IMAGE_ID] = {
		.path = BL33_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		.path = TRUSTED_BOOT_FW_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[TRUSTED_KEY_CERT_ID] = {
		.path = TRUSTED_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[SOC_FW_KEY_CERT_ID] = {
		.path = SOC_FW_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		.path = TOS_FW_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		.path = NT_FW_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		.path = SOC_FW_CONTENT_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		.path = TOS_FW_CONTENT_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		.path = NT_FW_CONTENT_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
#endif /* TRUSTED_BOARD_BOOT */
};



static int open_fip(const uintptr_t spec);
static int open_memmap(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, ARM platforms load images from the FIP */
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
	[BL32_EXTRA1_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		open_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		open_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		open_fip
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tb_fw_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&trusted_key_cert_uuid_spec,
		open_fip
	},
	[SOC_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_key_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_key_cert_uuid_spec,
		open_fip
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_key_cert_uuid_spec,
		open_fip
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_cert_uuid_spec,
		open_fip
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_cert_uuid_spec,
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

void plat_qemu_io_setup(void)
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

	/* Register the additional IO devices on this platform */
	io_result = register_io_dev_sh(&sh_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(sh_dev_con, (uintptr_t)NULL, &sh_dev_handle);
	assert(io_result == 0);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}

static int get_alt_image_source(unsigned int image_id, uintptr_t *dev_handle,
				  uintptr_t *image_spec)
{
	int result = open_semihosting((const uintptr_t)&sh_file_spec[image_id]);

	if (result == 0) {
		*dev_handle = sh_dev_handle;
		*image_spec = (uintptr_t)&sh_file_spec[image_id];
	}

	return result;
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
	} else {
		VERBOSE("Trying alternative IO\n");
		result = get_alt_image_source(image_id, dev_handle, image_spec);
	}

	return result;
}
