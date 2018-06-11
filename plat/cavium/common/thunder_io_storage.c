/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl_common.h>		/* For ARRAY_SIZE */
#include <debug.h>
#include <firmware_image_package.h>
#include <io_driver.h>
#include <io_fip.h>
#include <io_memmap.h>
#include <io_storage.h>
#include <platform_def.h>
#include <string.h>

#include <cavm-arch.h>

#define FIP_NAME "fip.bin"
#define ROMFS_NAME "ROM-FS"

/* IO devices */
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static io_block_spec_t fip_block_spec = {
	.offset	= 0x580000,
};

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t scp_bl2_uuid_spec = {
	.uuid = UUID_SCP_FIRMWARE_SCP_BL2,
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
static const io_uuid_spec_t tb_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FW_CERT,
};

static const io_uuid_spec_t trusted_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t scp_fw_key_cert_uuid_spec = {
	.uuid = UUID_SCP_FW_KEY_CERT,
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

static const io_uuid_spec_t scp_fw_cert_uuid_spec = {
	.uuid = UUID_SCP_FW_CONTENT_CERT,
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


static int open_fip(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, ARM platforms load images from the FIP */
static const struct plat_io_policy policies[] = {
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		open_fip
	},
	[SCP_BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_bl2_uuid_spec,
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
	[SCP_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_fw_key_cert_uuid_spec,
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
	[SCP_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_fw_cert_uuid_spec,
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
			VERBOSE("Using image from RAM\n");
			io_close(local_image_handle);
		}
	}
	return result;
}


void thunder_io_setup(void)
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

static uint16_t read_le16(const uint8_t *p)
{
	return ((((uint16_t) p[0]) << 0) |
		(((uint16_t) p[1]) << 8));
}

static uint32_t read_le32(const uint8_t *p)
{
	return ((((uint32_t) p[0]) << 0) |
		(((uint32_t) p[1]) << 8) |
		(((uint32_t) p[2]) << 16) |
		(((uint32_t) p[3]) << 24));
}

/* BDK HEADER (important part):
 * 0x00: jump instruction
 * 0x04: length of the image
 *
 * ROM-FS HEADER:
 * 0x00: "ROM-FS"
 * 0x06: file name length
 * 0x08: data length
 * 0x0c: file name
 * 0x0c + file_name: data
 */
static void plat_fill_fip_memmap_spec(void)
{
	size_t fip_name_len = strlen(FIP_NAME);
	/* We start from BDK image loaded */
	const uint8_t *ptr = (uint8_t *)0;

	ptr += read_le32(ptr + 4);
	while (memcmp(ptr, ROMFS_NAME, strlen(ROMFS_NAME)) == 0) {
		uint16_t fname_length = read_le16(ptr + 0x06);
		uint32_t fdata_length = read_le32(ptr + 0x08);
		const char *fname_ptr = (const char *)(ptr + 0x0c);
		const uint8_t *fdata_ptr = ptr + 0x0c + fname_length;

		if (fname_length == fip_name_len &&
		    strncasecmp(FIP_NAME, fname_ptr, fip_name_len) == 0) {
			fip_block_spec.offset = (size_t)fdata_ptr;
			return;
		}
		ptr = fdata_ptr + fdata_length;
	}
	ERROR("Could not find FIP image!!\n");
	fip_block_spec.offset = 0;
}

int plat_get_fip_source(uintptr_t *dev_handle, uintptr_t *image_spec)
{
	int result, boot_medium;
	cavm_gpio_strap_t gpio_strap;
	const char *medium;
	cavm_rst_boot_t rst_boot;

	int (*check)(const uintptr_t spec);
	uintptr_t handle;
	uintptr_t spec;

	rst_boot.u = CSR_READ_PA(0, CAVM_RST_BOOT);

	gpio_strap.u = CSR_READ_PA(0, CAVM_GPIO_STRAP);
	boot_medium = (gpio_strap.u) & 0x7;

	if (rst_boot.s.rboot) { /* Remote boot */
		plat_fill_fip_memmap_spec();
		handle = memmap_dev_handle;
		spec = (uintptr_t)&fip_block_spec;
		check = open_memmap;
		medium = "memmap";
	} else {
		ERROR("Boot medium %d not supported!\n", boot_medium);
		while(1);
	}

	result = check(spec);
	if (result == 0) {
		*image_spec = spec;
		*dev_handle = handle;
	} else {
		ERROR("FIP not found on medium %s\n", medium);
		result = -ENOENT;
	}

	return result;
}

/* Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	if (image_id == FIP_IMAGE_ID)
		return plat_get_fip_source(dev_handle, image_spec);

	policy = &policies[image_id];
	result = policy->check(policy->image_spec);

	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	} else {
		WARN("FIP not found\n");
		result = -ENOENT;
	}

	return result;
}
