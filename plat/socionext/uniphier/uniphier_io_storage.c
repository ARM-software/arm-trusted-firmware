/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <platform_def.h>

#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <tools_share/firmware_image_package.h>

#include "uniphier.h"

#define UNIPHIER_ROM_REGION_BASE	0x00000000ULL
#define UNIPHIER_ROM_REGION_SIZE	0x10000000ULL

#define UNIPHIER_OCM_REGION_BASE	0x30000000ULL
#define UNIPHIER_OCM_REGION_SIZE	0x00040000ULL

#define UNIPHIER_BLOCK_BUF_OFFSET	0x04200000UL
#define UNIPHIER_BLOCK_BUF_SIZE		0x00100000UL

static const io_dev_connector_t *uniphier_fip_dev_con;
static uintptr_t uniphier_fip_dev_handle;

static const io_dev_connector_t *uniphier_backend_dev_con;
static uintptr_t uniphier_backend_dev_handle;

static io_block_spec_t uniphier_fip_spec = {
	/* .offset will be set by the io_setup func */
	.length = 0x00200000,
};

static const io_uuid_spec_t uniphier_bl2_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t uniphier_scp_spec = {
	.uuid = UUID_SCP_FIRMWARE_SCP_BL2,
};

static const io_uuid_spec_t uniphier_bl31_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t uniphier_bl32_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t uniphier_bl33_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t uniphier_tb_fw_cert_spec = {
	.uuid = UUID_TRUSTED_BOOT_FW_CERT,
};

static const io_uuid_spec_t uniphier_trusted_key_cert_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t uniphier_scp_fw_key_cert_spec = {
	.uuid = UUID_SCP_FW_KEY_CERT,
};

static const io_uuid_spec_t uniphier_soc_fw_key_cert_spec = {
	.uuid = UUID_SOC_FW_KEY_CERT,
};

static const io_uuid_spec_t uniphier_tos_fw_key_cert_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t uniphier_nt_fw_key_cert_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t uniphier_scp_fw_cert_spec = {
	.uuid = UUID_SCP_FW_CONTENT_CERT,
};

static const io_uuid_spec_t uniphier_soc_fw_cert_spec = {
	.uuid = UUID_SOC_FW_CONTENT_CERT,
};

static const io_uuid_spec_t uniphier_tos_fw_cert_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t uniphier_nt_fw_cert_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif /* TRUSTED_BOARD_BOOT */

struct uniphier_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	uintptr_t init_params;
};

static const struct uniphier_io_policy uniphier_io_policies[] = {
	[FIP_IMAGE_ID] = {
		.dev_handle = &uniphier_backend_dev_handle,
		.image_spec = (uintptr_t)&uniphier_fip_spec,
	},
	[BL2_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_bl2_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SCP_BL2_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_scp_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL31_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_bl31_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL32_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_bl32_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL33_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_bl33_spec,
		.init_params = FIP_IMAGE_ID,
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_tb_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[TRUSTED_KEY_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_trusted_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SCP_FW_KEY_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_scp_fw_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SOC_FW_KEY_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_soc_fw_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_tos_fw_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_nt_fw_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SCP_FW_CONTENT_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_scp_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_soc_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_tos_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_nt_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
#endif
};

static int uniphier_io_block_setup(size_t fip_offset,
				   struct io_block_dev_spec *block_dev_spec,
				   size_t buffer_offset)
{
	int ret;

	uniphier_fip_spec.offset = fip_offset;

	block_dev_spec->buffer.offset = buffer_offset;
	block_dev_spec->buffer.length = UNIPHIER_BLOCK_BUF_SIZE;

	ret = mmap_add_dynamic_region(block_dev_spec->buffer.offset,
				      block_dev_spec->buffer.offset,
				      block_dev_spec->buffer.length,
				      MT_MEMORY | MT_RW | MT_NS);
	if (ret)
		return ret;

	ret = register_io_dev_block(&uniphier_backend_dev_con);
	if (ret)
		return ret;

	return io_dev_open(uniphier_backend_dev_con, (uintptr_t)block_dev_spec,
			   &uniphier_backend_dev_handle);
}

static int uniphier_io_memmap_setup(size_t fip_offset)
{
	int ret;

	uniphier_fip_spec.offset = fip_offset;

	ret = mmap_add_dynamic_region(fip_offset, fip_offset,
				      uniphier_fip_spec.length,
				      MT_RO_DATA | MT_SECURE);
	if (ret)
		return ret;

	ret = register_io_dev_memmap(&uniphier_backend_dev_con);
	if (ret)
		return ret;

	return io_dev_open(uniphier_backend_dev_con, 0,
			   &uniphier_backend_dev_handle);
}

static int uniphier_io_fip_setup(void)
{
	int ret;

	ret = register_io_dev_fip(&uniphier_fip_dev_con);
	if (ret)
		return ret;

	return io_dev_open(uniphier_fip_dev_con, 0, &uniphier_fip_dev_handle);
}

static int uniphier_io_emmc_setup(unsigned int soc, size_t buffer_offset)
{
	struct io_block_dev_spec *block_dev_spec;
	int ret;

	ret = uniphier_emmc_init(soc, &block_dev_spec);
	if (ret)
		return ret;

	return uniphier_io_block_setup(0x20000, block_dev_spec, buffer_offset);
}

static int uniphier_io_nand_setup(unsigned int soc, size_t buffer_offset)
{
	struct io_block_dev_spec *block_dev_spec;
	int ret;

	ret = uniphier_nand_init(soc, &block_dev_spec);
	if (ret)
		return ret;

	return uniphier_io_block_setup(0x20000, block_dev_spec, buffer_offset);
}

static int uniphier_io_nor_setup(unsigned int soc_id, size_t buffer_offset)
{
	return uniphier_io_memmap_setup(0x70000);
}

static int uniphier_io_usb_setup(unsigned int soc_id, size_t buffer_offset)
{
	struct io_block_dev_spec *block_dev_spec;
	int ret;

	/* use ROM API for loading images from USB storage */
	ret = mmap_add_dynamic_region(UNIPHIER_ROM_REGION_BASE,
				      UNIPHIER_ROM_REGION_BASE,
				      UNIPHIER_ROM_REGION_SIZE,
				      MT_CODE | MT_SECURE);
	if (ret)
		return ret;

	/*
	 * on-chip SRAM region: should be DEVICE attribute because the USB
	 * load functions provided by the ROM use this memory region as a work
	 * area, but do not cater to cache coherency.
	 */
	ret = mmap_add_dynamic_region(UNIPHIER_OCM_REGION_BASE,
				      UNIPHIER_OCM_REGION_BASE,
				      UNIPHIER_OCM_REGION_SIZE,
				      MT_DEVICE | MT_RW | MT_SECURE);
	if (ret)
		return ret;

	ret = uniphier_usb_init(soc_id, &block_dev_spec);
	if (ret)
		return ret;

	return uniphier_io_block_setup(0x20000, block_dev_spec, buffer_offset);
}

static int (* const uniphier_io_setup_table[])(unsigned int, size_t) = {
	[UNIPHIER_BOOT_DEVICE_EMMC] = uniphier_io_emmc_setup,
	[UNIPHIER_BOOT_DEVICE_NAND] = uniphier_io_nand_setup,
	[UNIPHIER_BOOT_DEVICE_NOR] = uniphier_io_nor_setup,
	[UNIPHIER_BOOT_DEVICE_USB] = uniphier_io_usb_setup,
};

int uniphier_io_setup(unsigned int soc_id, uintptr_t mem_base)
{
	int (*io_setup)(unsigned int soc_id, size_t buffer_offset);
	unsigned int boot_dev;
	int ret;

	boot_dev = uniphier_get_boot_device(soc_id);
	if (boot_dev == UNIPHIER_BOOT_DEVICE_RSV)
		return -EINVAL;

	io_setup = uniphier_io_setup_table[boot_dev];
	ret = io_setup(soc_id, mem_base + UNIPHIER_BLOCK_BUF_OFFSET);
	if (ret)
		return ret;

	ret = uniphier_io_fip_setup();
	if (ret)
		return ret;

	return 0;
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	uintptr_t init_params;

	assert(image_id < ARRAY_SIZE(uniphier_io_policies));

	*dev_handle = *uniphier_io_policies[image_id].dev_handle;
	*image_spec = uniphier_io_policies[image_id].image_spec;
	init_params = uniphier_io_policies[image_id].init_params;

	return io_dev_init(*dev_handle, init_params);
}
