/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <emmc.h>
#include <errno.h>
#include <firmware_image_package.h>
#include <io_block.h>
#include <io_driver.h>
#include <io_fip.h>
#include <io_memmap.h>
#include <io_storage.h>
#include <mmio.h>
#include <platform_def.h>
#include <semihosting.h>	/* For FOPEN_MODE_... */
#include <string.h>
#include "hikey_private.h"

#define EMMC_BLOCK_SHIFT			9

/* Page 1024, since only a few pages before 2048 are used as partition table */
#define SERIALNO_EMMC_OFFSET			(1024 * 512)

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static const io_dev_connector_t *emmc_dev_con, *emmc_rpmb_dev_con;
static uintptr_t emmc_dev_handle, emmc_rpmb_dev_handle;
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;

static int check_emmc(const uintptr_t spec);
static int check_fip(const uintptr_t spec);

static const io_block_spec_t emmc_fip_spec = {
	.offset		= HIKEY_FIP_BASE,
	.length		= HIKEY_FIP_MAX_SIZE,
};

static const io_block_spec_t emmc_rpmb_spec = {
	.offset		= HIKEY_NSBL1U_BASE,
	.length		= HIKEY_NSBL1U_MAX_SIZE,
};

static const io_block_spec_t emmc_user_data_spec = {
	.offset		= HIKEY_BL2U_BASE,
	.length		= HIKEY_BL2U_MAX_SIZE - EMMC_BLOCK_SIZE,
};

static const io_block_dev_spec_t emmc_dev_spec = {
	/* It's used as temp buffer in block driver. */
#if IMAGE_BL1
	.buffer		= {
		.offset	= HIKEY_BL1_MMC_DATA_BASE,
		.length	= HIKEY_BL1_MMC_DATA_SIZE,
	},
#else
	.buffer		= {
		.offset	= HIKEY_MMC_DATA_BASE,
		.length	= HIKEY_MMC_DATA_SIZE,
	},
#endif
	.ops		= {
		.read	= emmc_read_blocks,
		.write	= emmc_write_blocks,
	},
	.block_size	= EMMC_BLOCK_SIZE,
};

static const io_block_dev_spec_t emmc_rpmb_dev_spec = {
	.buffer		= {
		.offset	= HIKEY_MMC_DATA_BASE,
		.length	= HIKEY_MMC_DATA_SIZE,
	},
	.ops		= {
		.read	= emmc_rpmb_read_blocks,
		.write	= emmc_rpmb_write_blocks,
	},
	.block_size	= EMMC_BLOCK_SIZE,
};

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t bl31_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

static const io_uuid_spec_t scp_bl2_uuid_spec = {
	.uuid = UUID_SCP_FIRMWARE_SCP_BL2,
};

static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&emmc_dev_handle,
		(uintptr_t)&emmc_fip_spec,
		check_emmc
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		check_fip
	},
	[SCP_BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_bl2_uuid_spec,
		check_fip
	},
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		check_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		check_fip
	},
	[NS_BL1U_IMAGE_ID] = {
		&emmc_rpmb_dev_handle,
		(uintptr_t)&emmc_rpmb_spec,
		check_emmc
	},
	[BL2U_IMAGE_ID] = {
		&emmc_dev_handle,
		(uintptr_t)&emmc_user_data_spec,
		check_emmc
	}
};

static int check_emmc(const uintptr_t spec)
{
	int result;
	uintptr_t local_handle;

	result = io_dev_init(emmc_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(emmc_dev_handle, spec, &local_handle);
		if (result == 0) {
			io_close(local_handle);
		}
	}
	return result;
}

static int check_fip(const uintptr_t spec)
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

void hikey_io_setup(void)
{
	int result;

	result = register_io_dev_block(&emmc_dev_con);
	assert(result == 0);

	result = register_io_dev_block(&emmc_rpmb_dev_con);
	assert(result == 0);

	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	result = io_dev_open(emmc_dev_con, (uintptr_t)&emmc_dev_spec,
			     &emmc_dev_handle);
	assert(result == 0);

	result = io_dev_open(emmc_rpmb_dev_con, (uintptr_t)&emmc_rpmb_dev_spec,
			     &emmc_rpmb_dev_handle);
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)NULL, &fip_dev_handle);
	assert(result == 0);

	/* Ignore improbable errors in release builds */
	(void)result;
}

/* Return an IO device handle and specification which can be used to access
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
	assert(result == 0);

	*image_spec = policy->image_spec;
	*dev_handle = *(policy->dev_handle);

	return result;
}

static int hikey_read_block(unsigned int image_id, size_t offset, size_t size,
			    uintptr_t buffer)
{
	uintptr_t *dev_handle;
	uintptr_t img_handle, spec = 0;
	size_t bytes_read;
	io_block_spec_t *block_spec;
	int result;

	assert((image_id == BL2U_IMAGE_ID) || (image_id == NS_BL1U_IMAGE_ID));

	dev_handle = policies[image_id].dev_handle;
	assert((dev_handle == &emmc_dev_handle) ||
	       (dev_handle == &emmc_rpmb_dev_handle));
	result = plat_get_image_source(image_id, dev_handle, &spec);
	if (result) {
		WARN("Failed to get emmc area\n");
		return -ENODEV;
	}

	block_spec = (io_block_spec_t *)spec;
	/* Check whether address is in valid range. */
	assert((offset >= block_spec->offset) &&
	       (offset < block_spec->offset + block_spec->length) &&
	       (size <= block_spec->length) &&
	       (offset + size <= block_spec->offset + block_spec->length));
	result = io_open(*dev_handle, spec, &img_handle);
	if (result != 0) {
		WARN("Failed to open memmap device\n");
		return -ENODEV;
	}
	result = io_seek(img_handle, IO_SEEK_SET, offset);
	if (result) {
		WARN("Failed to seek at offset 0x%lx\n", offset);
		goto exit;
	}
	result = io_read(img_handle, buffer, size, &bytes_read);
	if ((result != 0) || (bytes_read < size)) {
		NOTICE("Failed to load data from 0x%lx\n", offset);
		goto exit;
	}
exit:
	io_close(img_handle);
	return result;
}

static int hikey_write_block(unsigned int image_id, size_t offset, size_t size,
			     uintptr_t buffer)
{
	uintptr_t *dev_handle;
	uintptr_t img_handle, spec = 0;
	size_t bytes_written;
	io_block_spec_t *block_spec;
	int result;

	assert((image_id == BL2U_IMAGE_ID) || (image_id == NS_BL1U_IMAGE_ID));

	dev_handle = policies[image_id].dev_handle;
	assert((dev_handle == &emmc_dev_handle) ||
	       (dev_handle == &emmc_rpmb_dev_handle));
	result = plat_get_image_source(image_id, dev_handle, &spec);
	if (result) {
		WARN("Failed to get emmc area\n");
		return -ENODEV;
	}
	(void)spec;

	block_spec = (io_block_spec_t *)spec;
	/* Check whether address is in valid range. */
	assert((offset >= block_spec->offset) &&
	       (offset < block_spec->offset + block_spec->length) &&
	       (size <= block_spec->length) &&
	       (offset + size <= block_spec->offset + block_spec->length));
	result = io_open(*dev_handle, spec, &img_handle);
	if (result != 0) {
		WARN("Failed to open memmap device\n");
		return -ENODEV;
	}
	result = io_seek(img_handle, IO_SEEK_SET, offset);
	if (result) {
		WARN("Failed to seek at offset 0x%lx\n", offset);
		goto exit;
	}
	result = io_write(img_handle, buffer, size, &bytes_written);
	if ((result != 0) || (bytes_written < size)) {
		NOTICE("Failed to write data into 0x%lx\n", offset);
		goto exit;
	}
exit:
	io_close(img_handle);
	return result;
}

int hikey_read_serialno(struct random_serial_num *serialno)
{
	int result;

	assert(serialno != NULL);
	result = hikey_read_block(BL2U_IMAGE_ID, SERIALNO_EMMC_OFFSET,
				  sizeof(struct random_serial_num),
				  (uintptr_t)serialno);
	assert(result == 0);
	/* Reference result. */
	(void)result;

	if (serialno->magic != RANDOM_MAGIC)
		return -ENOENT;
	return 0;
}

int hikey_write_serialno(struct random_serial_num *serialno)
{
	int result;

	assert((serialno != NULL) && (serialno->magic == RANDOM_MAGIC));
	result = hikey_write_block(BL2U_IMAGE_ID, SERIALNO_EMMC_OFFSET,
				   sizeof(struct random_serial_num),
				   (uintptr_t)serialno);
	assert(result == 0);
	return result;
}
