/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <firmware_image_package.h>
#include <io_block.h>
#include <io_driver.h>
#include <io_fip.h>
#include <io_memmap.h>
#include <io_storage.h>
#include <mmio.h>
#include <partition/partition.h>
#include <semihosting.h>
#include <string.h>
#include <tbbr_img_def.h>
#include <utils.h>
#include "platform_def.h"

static const io_dev_connector_t *mmap_dev_con;
static const io_dev_connector_t *fip_dev_con;

static uintptr_t mmap_dev_handle;
static uintptr_t fip_dev_handle;

static int open_mmap(const uintptr_t spec);
static int open_fip(const uintptr_t spec);

static const io_block_spec_t loader_fip_spec = {
	.offset		= FIP_BASE,
	.length		= FIP_SIZE
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

struct plat_io_policy {
	uintptr_t	*dev_handle;
	uintptr_t	image_spec;
	int		(*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&mmap_dev_handle,
		(uintptr_t)&loader_fip_spec,
		open_mmap
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
};

static int open_mmap(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(mmap_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(mmap_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			io_close(local_image_handle);
		}
	}
	return result;
}

static int open_fip(const uintptr_t spec)
{
	uintptr_t local_image_handle;
	int result;

	result = io_dev_init(fip_dev_handle, (uintptr_t) FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			io_close(local_image_handle);
		} else {
			VERBOSE("error opening fip\n");
		}
	} else {
		VERBOSE("error initializing fip\n");
	}

	return result;
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	const struct plat_io_policy *policy;
	int result;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	result = policy->check(policy->image_spec);
	assert(result == 0);

	*image_spec = policy->image_spec;
	*dev_handle = *(policy->dev_handle);

	return result;
}

void plat_io_setup(void)
{
	int result;

	result = register_io_dev_memmap(&mmap_dev_con);
	assert(result == 0);

	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)&loader_fip_spec,
				&fip_dev_handle);
	assert(result == 0);

	result = io_dev_open(mmap_dev_con, (uintptr_t)NULL, &mmap_dev_handle);
	assert(result == 0);

	(void) result;
}
