/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_image_package.h>

#include <plat_io_storage.h>

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static int open_memmap(const uintptr_t spec);
static int open_fip(const uintptr_t spec);

static uintptr_t fip_dev_handle;

static uintptr_t memmap_dev_handle;

static int open_memmap(const uintptr_t spec)
{
	uintptr_t temp_handle = 0U;
	int result;

	result = io_dev_init(memmap_dev_handle, (uintptr_t)0);
	if (result != 0) {
		return result;
	}

	result = io_open(memmap_dev_handle, spec, &temp_handle);
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
	static const io_dev_connector_t *memmap_dev_con;
	static const io_dev_connector_t *fip_dev_con;

	int result __unused;

	result = register_io_dev_memmap(&memmap_dev_con);
	assert(result == 0);

	result = io_dev_open(memmap_dev_con, (uintptr_t)0,
			     &memmap_dev_handle);
	assert(result == 0);

	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)0,
			     &fip_dev_handle);
	assert(result == 0);
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	static const io_block_spec_t fip_block_spec = {
		.offset = S32G_FIP_BASE,
		.length = S32G_FIP_SIZE,
	};

	static const io_uuid_spec_t bl31_uuid_spec = {
		.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
	};

	static const io_uuid_spec_t bl33_uuid_spec = {
		.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
	};

	static const struct plat_io_policy policies[BL33_IMAGE_ID + 1] = {
		[FIP_IMAGE_ID] = {
			.dev_handle = &memmap_dev_handle,
			.image_spec = (uintptr_t)&fip_block_spec,
			.check = open_memmap,
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
