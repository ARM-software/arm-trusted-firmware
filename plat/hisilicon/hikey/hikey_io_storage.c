/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_storage.h>
#include <drivers/mmc.h>
#include <drivers/partition/partition.h>
#include <lib/mmio.h>
#include <lib/semihosting.h>
#include <tools_share/firmware_image_package.h>

#include "hikey_private.h"

#define EMMC_BLOCK_SHIFT			9

/* Page 1024, since only a few pages before 2048 are used as partition table */
#define SERIALNO_EMMC_OFFSET			(1024 * 512)

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static const io_dev_connector_t *emmc_dev_con;
static uintptr_t emmc_dev_handle;
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;

static int check_emmc(const uintptr_t spec);
static int check_fip(const uintptr_t spec);

static io_block_spec_t emmc_fip_spec;

static const io_block_spec_t emmc_gpt_spec = {
	.offset		= 0,
	.length		= PLAT_PARTITION_BLOCK_SIZE *
			  (PLAT_PARTITION_MAX_ENTRIES / 4 + 2),
};

static const io_block_dev_spec_t emmc_dev_spec = {
	/* It's used as temp buffer in block driver. */
#ifdef IMAGE_BL1
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
		.read	= mmc_read_blocks,
		.write	= mmc_write_blocks,
	},
	.block_size	= MMC_BLOCK_SIZE,
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

static const io_uuid_spec_t scp_bl2_uuid_spec = {
	.uuid = UUID_SCP_FIRMWARE_SCP_BL2,
};

#if TRUSTED_BOARD_BOOT
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

static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&emmc_dev_handle,
		(uintptr_t)&emmc_fip_spec,
		check_emmc
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
	[BL32_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		check_fip
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		check_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		check_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		check_fip
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&trusted_key_cert_uuid_spec,
		check_fip
	},
	[SCP_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_fw_key_cert_uuid_spec,
		check_fip
	},
	[SOC_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_key_cert_uuid_spec,
		check_fip
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_key_cert_uuid_spec,
		check_fip
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_key_cert_uuid_spec,
		check_fip
	},
	[SCP_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_fw_cert_uuid_spec,
		check_fip
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_cert_uuid_spec,
		check_fip
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_cert_uuid_spec,
		check_fip
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_cert_uuid_spec,
		check_fip
	},
#endif /* TRUSTED_BOARD_BOOT */
	[GPT_IMAGE_ID] = {
		&emmc_dev_handle,
		(uintptr_t)&emmc_gpt_spec,
		check_emmc
	},
};

static int check_emmc(const uintptr_t spec)
{
	int result;
	uintptr_t local_handle;

	result = io_dev_init(emmc_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(emmc_dev_handle, spec, &local_handle);
		if (result == 0)
			io_close(local_handle);
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

	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	result = io_dev_open(emmc_dev_con, (uintptr_t)&emmc_dev_spec,
			     &emmc_dev_handle);
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)NULL, &fip_dev_handle);
	assert(result == 0);

	/* Ignore improbable errors in release builds */
	(void)result;
}

int hikey_set_fip_addr(unsigned int image_id, const char *name)
{
	const partition_entry_t *entry;

	if (emmc_fip_spec.length == 0) {
		partition_init(GPT_IMAGE_ID);
		entry = get_partition_entry(name);
		if (entry == NULL) {
			ERROR("Could NOT find the %s partition!\n", name);
			return -ENOENT;
		}
		emmc_fip_spec.offset = entry->start;
		emmc_fip_spec.length = entry->length;
	}
	return 0;
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
