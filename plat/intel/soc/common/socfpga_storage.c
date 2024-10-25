/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/cadence/cdns_nand.h>
#include <drivers/cadence/cdns_sdmmc.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_mtd.h>
#include <drivers/io/io_storage.h>
#include <drivers/mmc.h>
#include <drivers/partition/partition.h>
#include <lib/mmio.h>
#include <tools_share/firmware_image_package.h>

#include "drivers/sdmmc/sdmmc.h"
#include "socfpga_private.h"
#include "socfpga_ros.h"


static const io_dev_connector_t *fip_dev_con;
static const io_dev_connector_t *boot_dev_con;

static io_mtd_dev_spec_t nand_dev_spec;

static uintptr_t fip_dev_handle;
static uintptr_t boot_dev_handle;

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t bl31_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

# if ARM_LINUX_KERNEL_AS_BL33 != 0
static const io_uuid_spec_t nt_fw_config_uuid_spec = {
	.uuid = UUID_NT_FW_CONFIG,
};
# endif

uintptr_t a2_lba_offset;
const char a2[] = {0xa2, 0x0};

static const io_block_spec_t gpt_block_spec = {
	.offset = 0,
	.length = MMC_BLOCK_SIZE
};

static int check_fip(const uintptr_t spec);
static int check_dev(const uintptr_t spec);

static io_block_dev_spec_t boot_dev_spec;
static int (*register_io_dev)(const io_dev_connector_t **);

static io_block_spec_t fip_spec = {
	.offset		= PLAT_FIP_BASE,
	.length		= PLAT_FIP_MAX_SIZE,
};

struct plat_io_policy {
	uintptr_t       *dev_handle;
	uintptr_t       image_spec;
	int             (*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&boot_dev_handle,
		(uintptr_t)&fip_spec,
		check_dev
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
# if ARM_LINUX_KERNEL_AS_BL33 != 0
	[NT_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_config_uuid_spec,
		check_fip
	},
# endif
	[GPT_IMAGE_ID] = {
		&boot_dev_handle,
		(uintptr_t) &gpt_block_spec,
		check_dev
	},
};

static int check_dev(const uintptr_t spec)
{
	int result;
	uintptr_t local_handle;

	result = io_dev_init(boot_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(boot_dev_handle, spec, &local_handle);
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

void socfpga_io_setup(int boot_source, unsigned long offset)
{
	int result;
	fip_spec.offset = offset;

	switch (boot_source) {
	case BOOT_SOURCE_SDMMC:
		register_io_dev = &register_io_dev_block;
		boot_dev_spec.buffer.offset	= PLAT_MMC_DATA_BASE;
		boot_dev_spec.buffer.length	= SOCFPGA_MMC_BLOCK_SIZE;
		boot_dev_spec.ops.read		= SDMMC_READ_BLOCKS;
		boot_dev_spec.ops.write		= SDMMC_WRITE_BLOCKS;
		boot_dev_spec.block_size	= MMC_BLOCK_SIZE;
		break;

	case BOOT_SOURCE_QSPI:
		register_io_dev = &register_io_dev_memmap;
		break;

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	case BOOT_SOURCE_NAND:
		register_io_dev = &register_io_dev_mtd;
		nand_dev_spec.ops.init = cdns_nand_init_mtd;
		nand_dev_spec.ops.read = cdns_nand_read;
		nand_dev_spec.ops.write = NULL;
		break;
#endif

	default:
		ERROR("Unsupported boot source\n");
		panic();
		break;
	}

	result = (*register_io_dev)(&boot_dev_con);
	assert(result == 0);

	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	if (boot_source == BOOT_SOURCE_NAND) {
		result = io_dev_open(boot_dev_con, (uintptr_t)&nand_dev_spec,
								&boot_dev_handle);
	} else {
		result = io_dev_open(boot_dev_con, (uintptr_t)&boot_dev_spec,
								&boot_dev_handle);
	}
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)NULL, &fip_dev_handle);
	assert(result == 0);

	if (boot_source == BOOT_SOURCE_SDMMC) {
		partition_init(GPT_IMAGE_ID);
		fip_spec.offset = get_partition_entry(a2)->start;
	}

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
