/*
 * Copyright (c) 2022, Socionext Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>
#include <sq_common.h>

static const io_dev_connector_t *sq_fip_dev_con;
static uintptr_t sq_fip_dev_handle;

static const io_dev_connector_t *sq_backend_dev_con;
static uintptr_t sq_backend_dev_handle;

static io_block_spec_t sq_fip_spec = {
	.offset = PLAT_SQ_FIP_IOBASE,	/* FIP Image is at 5MB offset on memory-mapped NOR flash */
	.length = PLAT_SQ_FIP_MAXSIZE,	/* Expected maximum FIP image size */
};

static const io_uuid_spec_t sq_bl2_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t sq_bl31_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t sq_bl32_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t sq_bl33_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t sq_tb_fw_cert_spec = {
	.uuid = UUID_TRUSTED_BOOT_FW_CERT,
};

static const io_uuid_spec_t sq_trusted_key_cert_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t sq_soc_fw_key_cert_spec = {
	.uuid = UUID_SOC_FW_KEY_CERT,
};

static const io_uuid_spec_t sq_tos_fw_key_cert_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t sq_nt_fw_key_cert_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t sq_soc_fw_cert_spec = {
	.uuid = UUID_SOC_FW_CONTENT_CERT,
};

static const io_uuid_spec_t sq_tos_fw_cert_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t sq_nt_fw_cert_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif /* TRUSTED_BOARD_BOOT */

struct sq_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	uintptr_t init_params;
};

static const struct sq_io_policy sq_io_policies[] = {
	[FIP_IMAGE_ID] = {
		.dev_handle = &sq_backend_dev_handle,
		.image_spec = (uintptr_t)&sq_fip_spec,
	},
	[BL2_IMAGE_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_bl2_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL31_IMAGE_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_bl31_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL32_IMAGE_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_bl32_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL33_IMAGE_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_bl33_spec,
		.init_params = FIP_IMAGE_ID,
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_tb_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[TRUSTED_KEY_CERT_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_trusted_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SOC_FW_KEY_CERT_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_soc_fw_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_tos_fw_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_nt_fw_key_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_soc_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_tos_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		.dev_handle = &sq_fip_dev_handle,
		.image_spec = (uintptr_t)&sq_nt_fw_cert_spec,
		.init_params = FIP_IMAGE_ID,
	},
#endif
};

static int sq_update_fip_spec(void)
{
	uint32_t boot_index;
	int ret;

	ret = mmap_add_dynamic_region(PLAT_SQ_BOOTIDX_BASE, PLAT_SQ_BOOTIDX_BASE,
				      PAGE_SIZE, MT_RO_DATA | MT_SECURE);
	if (ret) {
		return ret;
	}

	boot_index = mmio_read_32(PLAT_SQ_BOOTIDX_BASE);
	if (boot_index < PLAT_SQ_MAX_BOOT_INDEX) {
		sq_fip_spec.offset += PLAT_SQ_FIP_MAXSIZE * boot_index;
		INFO("FWU Enabled: boot_index %d\n", boot_index);
	} else {
		WARN("FWU Disabled: wrong boot_index value. Fallback to index 0.\n");
	}

	mmap_remove_dynamic_region(PLAT_SQ_BOOTIDX_BASE, PAGE_SIZE);
	return 0;
}

static int sq_io_memmap_setup(void)
{
	int ret;

	ret = sq_update_fip_spec();
	if (ret) {
		return ret;
	}

	ret = mmap_add_dynamic_region(sq_fip_spec.offset, sq_fip_spec.offset,
				      sq_fip_spec.length, MT_RO_DATA | MT_SECURE);
	if (ret) {
		return ret;
	}

	ret = register_io_dev_memmap(&sq_backend_dev_con);
	if (ret) {
		return ret;
	}

	return io_dev_open(sq_backend_dev_con, 0, &sq_backend_dev_handle);
}

static int sq_io_fip_setup(void)
{
	int ret;

	ret = register_io_dev_fip(&sq_fip_dev_con);
	if (ret) {
		return ret;
	}

	return io_dev_open(sq_fip_dev_con, 0, &sq_fip_dev_handle);
}

int sq_io_setup(void)
{
	int ret;

	ret = sq_io_memmap_setup();
	if (ret) {
		return ret;
	}

	ret = sq_io_fip_setup();
	if (ret) {
		return ret;
	}

	return 0;
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	uintptr_t init_params;

	assert(image_id < ARRAY_SIZE(sq_io_policies));

	*dev_handle = *sq_io_policies[image_id].dev_handle;
	*image_spec = sq_io_policies[image_id].image_spec;
	init_params = sq_io_policies[image_id].init_params;

	return io_dev_init(*dev_handle, init_params);
}
