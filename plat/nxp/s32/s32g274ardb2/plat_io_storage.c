/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/mmc.h>
#include <drivers/partition/partition.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_image_package.h>

#include <plat_io_storage.h>

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static uintptr_t fip_dev_handle;

static io_block_spec_t fip_mmc_spec;

static uintptr_t mmc_dev_handle;

static int open_mmc(const uintptr_t spec)
{
	uintptr_t temp_handle;
	int result;

	result = io_dev_init(mmc_dev_handle, (uintptr_t)0U);
	if (result != 0) {
		return result;
	}

	result = io_open(mmc_dev_handle, spec, &temp_handle);
	if (result == 0) {
		(void)io_close(temp_handle);
	}

	return result;
}

static int open_fip(const uintptr_t spec)
{
	uintptr_t temp_handle = 0U;
	int result;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result != 0) {
		return result;
	}

	result = io_open(fip_dev_handle, spec, &temp_handle);
	if (result == 0) {
		(void)io_close(temp_handle);
	}

	return result;
}

void plat_s32g2_io_setup(void)
{
	static const io_block_dev_spec_t mmc_dev_spec = {
		/* It's used as temp buffer in block driver. */
		.buffer		= {
			.offset = IO_BUFFER_BASE,
			.length = IO_BUFFER_SIZE,
		},
		.ops		= {
			.read	= mmc_read_blocks,
			.write	= mmc_write_blocks,
		},
		.block_size	= MMC_BLOCK_SIZE,
	};
	static const io_dev_connector_t *fip_dev_con;
	static const io_dev_connector_t *mmc_dev_con;

	partition_entry_t fip_part;
	uintptr_t io_buf_base;
	int result __unused;
	size_t io_buf_size;
	int ret;

	io_buf_base = mmc_dev_spec.buffer.offset;
	io_buf_size = mmc_dev_spec.buffer.length;

	ret = mmap_add_dynamic_region(io_buf_base, io_buf_base,
				      io_buf_size,
				      MT_MEMORY | MT_RW | MT_SECURE);
	if (ret != 0) {
		ERROR("Failed to map the IO buffer\n");
		panic();
	}

	result = register_io_dev_block(&mmc_dev_con);
	assert(result == 0);

	result = io_dev_open(mmc_dev_con, (uintptr_t)&mmc_dev_spec,
			     &mmc_dev_handle);
	assert(result == 0);

	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)0,
			     &fip_dev_handle);
	assert(result == 0);

	ret = gpt_partition_init();
	if (ret != 0) {
		ERROR("Could not load MBR partition table\n");
		panic();
	}

	fip_part = get_partition_entry_list()->list[FIP_PART];
	fip_mmc_spec.offset = fip_part.start;
	fip_mmc_spec.length = fip_part.length;
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	static const io_block_spec_t mbr_spec = {
		.offset = 0,
		.length = PLAT_PARTITION_BLOCK_SIZE,
	};

	static const io_uuid_spec_t bl31_uuid_spec = {
		.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
	};

	static const io_uuid_spec_t bl33_uuid_spec = {
		.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
	};

	static const struct plat_io_policy policies[GPT_IMAGE_ID + 1] = {
		[FIP_IMAGE_ID] = {
			.dev_handle = &mmc_dev_handle,
			.image_spec = (uintptr_t)&fip_mmc_spec,
			.check = open_mmc,
		},
		[BL31_IMAGE_ID] = {
			.dev_handle = &fip_dev_handle,
			.image_spec = (uintptr_t)&bl31_uuid_spec,
			.check = open_fip,
		},
		[BL33_IMAGE_ID] = {
			.dev_handle = &fip_dev_handle,
			.image_spec = (uintptr_t)&bl33_uuid_spec,
			.check = open_fip,
		},
		[GPT_IMAGE_ID] = {
			.dev_handle = &mmc_dev_handle,
			.image_spec = (uintptr_t)&mbr_spec,
			.check = open_mmc,
		},
	};
	const struct plat_io_policy *policy;
	int result;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	result = policy->check(policy->image_spec);
	assert(result == 0);

	*image_spec = policy->image_spec;
	*dev_handle = *policy->dev_handle;

	return result;
}
