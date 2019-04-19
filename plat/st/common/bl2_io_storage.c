/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_dummy.h>
#include <drivers/io/io_storage.h>
#include <drivers/mmc.h>
#include <drivers/partition/partition.h>
#include <drivers/st/io_mmc.h>
#include <drivers/st/io_stm32image.h>
#include <drivers/st/stm32_sdmmc2.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

/* IO devices */
static const io_dev_connector_t *dummy_dev_con;
static uintptr_t dummy_dev_handle;
static uintptr_t dummy_dev_spec;

static uintptr_t image_dev_handle;

static io_block_spec_t gpt_block_spec = {
	.offset = 0,
	.length = 34 * MMC_BLOCK_SIZE, /* Size of GPT table */
};

static uint32_t block_buffer[MMC_BLOCK_SIZE] __aligned(MMC_BLOCK_SIZE);

static const io_block_dev_spec_t mmc_block_dev_spec = {
	/* It's used as temp buffer in block driver */
	.buffer = {
		.offset = (size_t)&block_buffer,
		.length = MMC_BLOCK_SIZE,
	},
	.ops = {
		.read = mmc_read_blocks,
		.write = NULL,
	},
	.block_size = MMC_BLOCK_SIZE,
};

static uintptr_t storage_dev_handle;
static const io_dev_connector_t *mmc_dev_con;

#ifdef AARCH32_SP_OPTEE
static const struct stm32image_part_info optee_header_partition_spec = {
	.name = OPTEE_HEADER_IMAGE_NAME,
	.binary_type = OPTEE_HEADER_BINARY_TYPE,
};

static const struct stm32image_part_info optee_pager_partition_spec = {
	.name = OPTEE_PAGER_IMAGE_NAME,
	.binary_type = OPTEE_PAGER_BINARY_TYPE,
};

static const struct stm32image_part_info optee_paged_partition_spec = {
	.name = OPTEE_PAGED_IMAGE_NAME,
	.binary_type = OPTEE_PAGED_BINARY_TYPE,
};
#else
static const io_block_spec_t bl32_block_spec = {
	.offset = BL32_BASE,
	.length = STM32MP_BL32_SIZE
};
#endif

static const io_block_spec_t bl2_block_spec = {
	.offset = BL2_BASE,
	.length = STM32MP_BL2_SIZE,
};

static const struct stm32image_part_info bl33_partition_spec = {
	.name = BL33_IMAGE_NAME,
	.binary_type = BL33_BINARY_TYPE,
};

enum {
	IMG_IDX_BL33,
#ifdef AARCH32_SP_OPTEE
	IMG_IDX_OPTEE_HEADER,
	IMG_IDX_OPTEE_PAGER,
	IMG_IDX_OPTEE_PAGED,
#endif
	IMG_IDX_NUM
};

static struct stm32image_device_info stm32image_dev_info_spec = {
	.lba_size = MMC_BLOCK_SIZE,
	.part_info[IMG_IDX_BL33] = {
		.name = BL33_IMAGE_NAME,
		.binary_type = BL33_BINARY_TYPE,
	},
#ifdef AARCH32_SP_OPTEE
	.part_info[IMG_IDX_OPTEE_HEADER] = {
		.name = OPTEE_HEADER_IMAGE_NAME,
		.binary_type = OPTEE_HEADER_BINARY_TYPE,
	},
	.part_info[IMG_IDX_OPTEE_PAGER] = {
		.name = OPTEE_PAGER_IMAGE_NAME,
		.binary_type = OPTEE_PAGER_BINARY_TYPE,
	},
	.part_info[IMG_IDX_OPTEE_PAGED] = {
		.name = OPTEE_PAGED_IMAGE_NAME,
		.binary_type = OPTEE_PAGED_BINARY_TYPE,
	},
#endif
};

static io_block_spec_t stm32image_block_spec = {
	.offset = 0,
	.length = 0,
};

static const io_dev_connector_t *stm32image_dev_con;

static int open_dummy(const uintptr_t spec);
static int open_image(const uintptr_t spec);
static int open_storage(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	[BL2_IMAGE_ID] = {
		.dev_handle = &dummy_dev_handle,
		.image_spec = (uintptr_t)&bl2_block_spec,
		.check = open_dummy
	},
#ifdef AARCH32_SP_OPTEE
	[BL32_IMAGE_ID] = {
		.dev_handle = &image_dev_handle,
		.image_spec = (uintptr_t)&optee_header_partition_spec,
		.check = open_image
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		.dev_handle = &image_dev_handle,
		.image_spec = (uintptr_t)&optee_pager_partition_spec,
		.check = open_image
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		.dev_handle = &image_dev_handle,
		.image_spec = (uintptr_t)&optee_paged_partition_spec,
		.check = open_image
	},
#else
	[BL32_IMAGE_ID] = {
		.dev_handle = &dummy_dev_handle,
		.image_spec = (uintptr_t)&bl32_block_spec,
		.check = open_dummy
	},
#endif
	[BL33_IMAGE_ID] = {
		.dev_handle = &image_dev_handle,
		.image_spec = (uintptr_t)&bl33_partition_spec,
		.check = open_image
	},
	[GPT_IMAGE_ID] = {
		.dev_handle = &storage_dev_handle,
		.image_spec = (uintptr_t)&gpt_block_spec,
		.check = open_storage
	},
	[STM32_IMAGE_ID] = {
		.dev_handle = &storage_dev_handle,
		.image_spec = (uintptr_t)&stm32image_block_spec,
		.check = open_storage
	}
};

static int open_dummy(const uintptr_t spec)
{
	return io_dev_init(dummy_dev_handle, 0);
}

static int open_image(const uintptr_t spec)
{
	return io_dev_init(image_dev_handle, 0);
}

static int open_storage(const uintptr_t spec)
{
	return io_dev_init(storage_dev_handle, 0);
}

static void print_boot_device(boot_api_context_t *boot_context)
{
	switch (boot_context->boot_interface_selected) {
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_SD:
		INFO("Using SDMMC\n");
		break;
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_EMMC:
		INFO("Using EMMC\n");
		break;
	default:
		ERROR("Boot interface not found\n");
		panic();
		break;
	}

	if (boot_context->boot_interface_instance != 0U) {
		INFO("  Instance %d\n", boot_context->boot_interface_instance);
	}
}

static void boot_mmc(enum mmc_device_type mmc_dev_type,
		     uint16_t boot_interface_instance)
{
	int io_result __unused;
	uint8_t idx;
	struct stm32image_part_info *part;
	struct stm32_sdmmc2_params params;
	struct mmc_device_info device_info;
	const partition_entry_t *entry;

	zeromem(&device_info, sizeof(struct mmc_device_info));
	zeromem(&params, sizeof(struct stm32_sdmmc2_params));

	device_info.mmc_dev_type = mmc_dev_type;

	switch (boot_interface_instance) {
	case 1:
		params.reg_base = STM32MP_SDMMC1_BASE;
		break;
	case 2:
		params.reg_base = STM32MP_SDMMC2_BASE;
		break;
	case 3:
		params.reg_base = STM32MP_SDMMC3_BASE;
		break;
	default:
		WARN("SDMMC instance not found, using default\n");
		if (mmc_dev_type == MMC_IS_SD) {
			params.reg_base = STM32MP_SDMMC1_BASE;
		} else {
			params.reg_base = STM32MP_SDMMC2_BASE;
		}
		break;
	}

	params.device_info = &device_info;
	if (stm32_sdmmc2_mmc_init(&params) != 0) {
		ERROR("SDMMC%u init failed\n", boot_interface_instance);
		panic();
	}

	/* Open MMC as a block device to read GPT table */
	io_result = register_io_dev_block(&mmc_dev_con);
	if (io_result != 0) {
		panic();
	}

	io_result = io_dev_open(mmc_dev_con, (uintptr_t)&mmc_block_dev_spec,
				&storage_dev_handle);
	assert(io_result == 0);

	partition_init(GPT_IMAGE_ID);

	io_result = io_dev_close(storage_dev_handle);
	assert(io_result == 0);

	stm32image_dev_info_spec.device_size =
		stm32_sdmmc2_mmc_get_device_size();

	for (idx = 0U; idx < IMG_IDX_NUM; idx++) {
		part = &stm32image_dev_info_spec.part_info[idx];
		entry = get_partition_entry(part->name);
		if (entry == NULL) {
			ERROR("Partition %s not found\n", part->name);
			panic();
		}

		part->part_offset = entry->start;
		part->bkp_offset = 0U;
	}

	/*
	 * Re-open MMC with io_mmc, for better perfs compared to
	 * io_block.
	 */
	io_result = register_io_dev_mmc(&mmc_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(mmc_dev_con, 0, &storage_dev_handle);
	assert(io_result == 0);

	io_result = register_io_dev_stm32image(&stm32image_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(stm32image_dev_con,
				(uintptr_t)&stm32image_dev_info_spec,
				&image_dev_handle);
	assert(io_result == 0);
}

void stm32mp_io_setup(void)
{
	int io_result __unused;
	boot_api_context_t *boot_context =
		(boot_api_context_t *)stm32mp_get_boot_ctx_address();

	print_boot_device(boot_context);

	if ((boot_context->boot_partition_used_toboot == 1U) ||
	    (boot_context->boot_partition_used_toboot == 2U)) {
		INFO("Boot used partition fsbl%d\n",
		     boot_context->boot_partition_used_toboot);
	}

	io_result = register_io_dev_dummy(&dummy_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(dummy_dev_con, dummy_dev_spec,
				&dummy_dev_handle);
	assert(io_result == 0);

	switch (boot_context->boot_interface_selected) {
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_SD:
		dmbsy();
		boot_mmc(MMC_IS_SD, boot_context->boot_interface_instance);
		break;
	case BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_EMMC:
		dmbsy();
		boot_mmc(MMC_IS_EMMC, boot_context->boot_interface_instance);
		break;

	default:
		ERROR("Boot interface %d not supported\n",
		      boot_context->boot_interface_selected);
		break;
	}
}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy.
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int rc;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	rc = policy->check(policy->image_spec);
	if (rc == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	}

	return rc;
}
