/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/mmc.h>
#include <tools_share/firmware_image_package.h>

static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;

#ifndef IMX7_FIP_MMAP
static const io_dev_connector_t *mmc_dev_con;
static uintptr_t mmc_dev_handle;

static const io_block_spec_t mmc_fip_spec = {
	.offset = IMX7_FIP_MMC_BASE,
	.length = IMX7_FIP_SIZE
};

static const io_block_dev_spec_t mmc_dev_spec = {
	/* It's used as temp buffer in block driver. */
	.buffer		= {
		.offset	= IMX7_FIP_BASE,
		/* do we need a new value? */
		.length = IMX7_FIP_SIZE
	},
	.ops		= {
		.read	= mmc_read_blocks,
		.write	= mmc_write_blocks,
	},
	.block_size	= MMC_BLOCK_SIZE,
};

static int open_mmc(const uintptr_t spec);

#else
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static const io_block_spec_t fip_block_spec = {
	.offset = IMX7_FIP_BASE,
	.length = IMX7_FIP_SIZE
};
static int open_memmap(const uintptr_t spec);
#endif
static int open_fip(const uintptr_t spec);

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

static const io_uuid_spec_t tos_fw_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t tos_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t nt_fw_key_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t nt_fw_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif /* TRUSTED_BOARD_BOOT */

/* TODO: this structure is replicated multiple times. rationalize it ! */
struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
#ifndef IMX7_FIP_MMAP
	[FIP_IMAGE_ID] = {
		&mmc_dev_handle,
		(uintptr_t)&mmc_fip_spec,
		open_mmc
	},
#else
	[FIP_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_memmap
	},
#endif
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

#ifndef IMX7_FIP_MMAP
static int open_mmc(const uintptr_t spec)
{
	int result;
	uintptr_t local_handle;

	result = io_dev_init(mmc_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(mmc_dev_handle, spec, &local_handle);
		if (result == 0)
			io_close(local_handle);
	}
	return result;
}
#else
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
#endif

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	result = policy->check(policy->image_spec);
	assert(result == 0);

	*image_spec = policy->image_spec;
	*dev_handle = *policy->dev_handle;

	return result;
}

void plat_imx7_io_setup(void)
{
	int result __unused;

#ifndef IMX7_FIP_MMAP
	result = register_io_dev_block(&mmc_dev_con);
	assert(result == 0);

	result = io_dev_open(mmc_dev_con, (uintptr_t)&mmc_dev_spec,
			     &mmc_dev_handle);
	assert(result == 0);

#else
	result = register_io_dev_memmap(&memmap_dev_con);
	assert(result == 0);

	result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
			     &memmap_dev_handle);
	assert(result == 0);

#endif
	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
			     &fip_dev_handle);
	assert(result == 0);
}
