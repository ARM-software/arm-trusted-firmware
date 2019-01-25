/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <drivers/mmc.h>
#include <tools_share/firmware_image_package.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_storage.h>
#include <lib/mmio.h>
#include <drivers/partition/partition.h>
#include <lib/semihosting.h>
#include <string.h>
#include <lib/utils.h>
#include <common/tbbr/tbbr_img_def.h>
#include "platform_def.h"

#define STRATIX10_FIP_BASE	(0)
#define STRATIX10_FIP_MAX_SIZE	(0x1000000)
#define STRATIX10_MMC_DATA_BASE	(0x400000)
#define STRATIX10_MMC_DATA_SIZE	(0x1000000)

static const io_dev_connector_t *mmc_dev_con;
static const io_dev_connector_t *fip_dev_con;

static uintptr_t fip_dev_handle;
static uintptr_t mmc_dev_handle;

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t bl31_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

uintptr_t a2_lba_offset;

static const io_block_spec_t gpt_block_spec = {
	.offset = 0,
	.length = MMC_BLOCK_SIZE
};

static int check_mmc(const uintptr_t spec);
static int check_fip(const uintptr_t spec);

static io_block_spec_t mmc_fip_spec = {
	.offset		= STRATIX10_FIP_BASE,
	.length		= STRATIX10_FIP_MAX_SIZE,
};

const char a2[] = {0xa2, 0x0};

static const io_block_dev_spec_t mmc_dev_spec = {
	.buffer	= {
		.offset = STRATIX10_MMC_DATA_BASE, // change this to a2 base
		.length = MMC_BLOCK_SIZE, // this is the file size of fip
	},

	.ops	= {
		.read	= mmc_read_blocks,
		.write	= mmc_write_blocks,
	},

	.block_size = MMC_BLOCK_SIZE,
};

struct plat_io_policy {
	uintptr_t	*dev_handle;
	uintptr_t	image_spec;
	int		(*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&mmc_dev_handle,
		(uintptr_t)&mmc_fip_spec,
		check_mmc
	},
	[BL2_IMAGE_ID] = {
	  &fip_dev_handle,
	  (uintptr_t)&bl2_uuid_spec,
	  check_fip
	},
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		check_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t) &bl33_uuid_spec,
		check_fip
	},
	[GPT_IMAGE_ID] = {
		&mmc_dev_handle,
		(uintptr_t) &gpt_block_spec,
		check_mmc
	},
};

static int check_mmc(const uintptr_t spec)
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

static int check_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0)
			io_close(local_image_handle);
	}
	return result;
}

void stratix10_io_setup(void)
{
	int result;

	result = register_io_dev_block(&mmc_dev_con);
	assert(result == 0);

	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	result = io_dev_open(mmc_dev_con, (uintptr_t)&mmc_dev_spec,
			&mmc_dev_handle);
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)NULL, &fip_dev_handle);
	assert(result == 0);

	partition_init(GPT_IMAGE_ID);

	mmc_fip_spec.offset = get_partition_entry(a2)->start;

	(void)result;
}

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
	*dev_handle = *(policy->dev_handle);

	return result;
}
