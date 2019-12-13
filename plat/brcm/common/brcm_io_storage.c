/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_storage.h>
#include <tools_share/firmware_image_package.h>

#include <cmn_plat_def.h>
#include <cmn_plat_util.h>
#include <plat_brcm.h>
#include <platform_def.h>

/* IO devices */
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static const io_block_spec_t fip_block_spec = {
	.offset = PLAT_BRCM_FIP_BASE,
	.length = PLAT_BRCM_FIP_MAX_SIZE
};

static const io_block_spec_t qspi_fip_block_spec = {
	.offset = PLAT_BRCM_FIP_QSPI_BASE,
	.length = PLAT_BRCM_FIP_MAX_SIZE
};

static const io_block_spec_t nand_fip_block_spec = {
	.offset = PLAT_BRCM_FIP_NAND_BASE,
	.length = PLAT_BRCM_FIP_MAX_SIZE
};

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t scp_bl2_uuid_spec = {
	.uuid = UUID_SCP_FIRMWARE_SCP_BL2,
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

static const io_uuid_spec_t tb_fw_config_uuid_spec = {
	.uuid = UUID_TB_FW_CONFIG,
};

static const io_uuid_spec_t hw_config_uuid_spec = {
	.uuid = UUID_HW_CONFIG,
};

static const io_uuid_spec_t soc_fw_config_uuid_spec = {
	.uuid = UUID_SOC_FW_CONFIG,
};

static const io_uuid_spec_t tos_fw_config_uuid_spec = {
	.uuid = UUID_TOS_FW_CONFIG,
};

static const io_uuid_spec_t nt_fw_config_uuid_spec = {
	.uuid = UUID_NT_FW_CONFIG,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t tb_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FW_CERT,
};

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

static int open_fip(const uintptr_t spec);
static int open_memmap(const uintptr_t spec);
static int open_qspi(const uintptr_t spec);
static int open_nand(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, BRCM platforms load images from the FIP */
static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_memmap
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		open_fip
	},
	[SCP_BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_bl2_uuid_spec,
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
	[BL32_EXTRA1_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		open_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		open_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		open_fip
	},
	[TB_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tb_fw_config_uuid_spec,
		open_fip
	},
	[HW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&hw_config_uuid_spec,
		open_fip
	},
	[SOC_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_config_uuid_spec,
		open_fip
	},
	[TOS_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_config_uuid_spec,
		open_fip
	},
	[NT_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_config_uuid_spec,
		open_fip
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tb_fw_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&trusted_key_cert_uuid_spec,
		open_fip
	},
	[SCP_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_fw_key_cert_uuid_spec,
		open_fip
	},
	[SOC_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_key_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_key_cert_uuid_spec,
		open_fip
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_key_cert_uuid_spec,
		open_fip
	},
	[SCP_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&scp_fw_cert_uuid_spec,
		open_fip
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_cert_uuid_spec,
		open_fip
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_cert_uuid_spec,
		open_fip
	},
#endif /* TRUSTED_BOARD_BOOT */
};

/* By default, BRCM platforms load images from the FIP */
static const struct plat_io_policy boot_source_policies[] = {
	[BOOT_SOURCE_QSPI] = {
		&memmap_dev_handle,
		(uintptr_t)&qspi_fip_block_spec,
		open_qspi
	},
	[BOOT_SOURCE_NAND] = {
		&memmap_dev_handle,
		(uintptr_t)&nand_fip_block_spec,
		open_nand
	},
};

/* Weak definitions may be overridden in specific brcm platform */
#pragma weak plat_brcm_io_setup
#pragma weak plat_brcm_process_flags

static int open_fip(const uintptr_t spec)
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


static int open_memmap(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(memmap_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(memmap_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using Memmap\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

static int open_qspi(const uintptr_t spec)
{
	return open_memmap(spec);
}

static int open_nand(const uintptr_t spec)
{
	return open_memmap(spec);
}


void brcm_io_setup(void)
{
	int io_result;
	uint32_t boot_source;

	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == 0);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	assert(io_result == 0);

	boot_source = boot_source_get();
	switch (boot_source) {
	case BOOT_SOURCE_QSPI:
	case BOOT_SOURCE_NAND:
	default:
		io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
					&memmap_dev_handle);
		break;
	}
	assert(io_result == 0);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}

void plat_brcm_io_setup(void)
{
	brcm_io_setup();
}

void plat_brcm_process_flags(uint16_t plat_toc_flags __unused)
{
	WARN("%s not implemented\n", __func__);
}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;
	uint32_t boot_source;
	uint16_t lcl_plat_toc_flg;

	assert(image_id < ARRAY_SIZE(policies));

	boot_source = boot_source_get();
	if (image_id == FIP_IMAGE_ID)
		policy = &boot_source_policies[boot_source];
	else
		policy = &policies[image_id];

	result = policy->check(policy->image_spec);
	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);

		if (image_id == TRUSTED_BOOT_FW_CERT_ID) {
			/*
			 * Process the header flags to perform
			 * such custom actions as speeding up PLL.
			 * CERT seems to be the first image accessed
			 * by BL1 so this is where we process the flags.
			 */
			fip_dev_get_plat_toc_flag((io_dev_info_t *)fip_dev_handle,
						  &lcl_plat_toc_flg);
			plat_brcm_process_flags(lcl_plat_toc_flg);
		}
	}

	return result;
}
