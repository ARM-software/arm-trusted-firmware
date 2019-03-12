/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/io/io_block.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_storage.h>
#include <endian.h>
#include <lib/utils.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <tools_share/firmware_image_package.h>

#include <io.h>
#include <plat_common.h>

uint32_t fip_device;
/* IO devices */
uintptr_t backend_dev_handle;

static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;
static const io_dev_connector_t *backend_dev_con;

static io_block_spec_t fip_block_spec = {
	.offset = PLAT_FIP_OFFSET,
	.length = PLAT_FIP_MAX_SIZE
};

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t fuse_bl2_uuid_spec = {
	.uuid = UUID_SCP_FIRMWARE_SCP_BL2,
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

static const io_uuid_spec_t tb_fw_config_uuid_spec = {
	.uuid = UUID_TB_FW_CONFIG,
};

static const io_uuid_spec_t hw_config_uuid_spec = {
	.uuid = UUID_HW_CONFIG,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t tb_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FW_CERT,
};

static const io_uuid_spec_t trusted_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t fuse_key_cert_uuid_spec = {
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

static const io_uuid_spec_t fuse_cert_uuid_spec = {
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

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, ARM platforms load images from the FIP */
static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&backend_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_backend
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		open_fip
	},
	[SCP_BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&fuse_bl2_uuid_spec,
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
		(uintptr_t)&fuse_key_cert_uuid_spec,
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
		(uintptr_t)&fuse_cert_uuid_spec,
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


/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_io_setup
#pragma weak plat_get_alt_image_source
#pragma weak plat_get_fuse_image_source
#pragma weak ddr_fip_setup
#pragma weak fuse_fip_setup


int ddr_fip_setup(const io_dev_connector_t *fip_dev_con, unsigned int boot_dev)
{
	/* By default no ddr FIP image*/
	return 0;
}

int fuse_fip_setup(const io_dev_connector_t *fip_dev_con, unsigned int boot_dev)
{
	/* By default no ddr FIP image*/
	return 0;
}

/*
 * Return an IO device handle and specification which can be used to access
 */
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


int open_backend(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	result = io_dev_init(backend_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(backend_dev_handle, spec, &local_image_handle);
		if (result == 0)
			io_close(local_image_handle);
	}
	return result;
}

int plat_io_block_setup(size_t fip_offset, uintptr_t block_dev_spec)
{
	int io_result;

	fip_block_spec.offset = fip_offset;

	io_result = register_io_dev_block(&backend_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(backend_dev_con, block_dev_spec,
				&backend_dev_handle);
	assert(io_result == 0);

	return io_result;
}

int plat_io_memmap_setup(size_t fip_offset)
{
	int io_result;

	fip_block_spec.offset = fip_offset;

	io_result = register_io_dev_memmap(&backend_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(backend_dev_con, (uintptr_t)NULL,
				&backend_dev_handle);
	assert(io_result == 0);

	return io_result;
}

static int ls_io_fip_setup(unsigned int boot_dev)
{
	int io_result;

	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)&fip_device,
				&fip_dev_handle);
	assert(io_result == 0);

	/* Open connection to DDR FIP image if available */
	io_result = ddr_fip_setup(fip_dev_con, boot_dev);

	assert(io_result == 0);

	/* Open connection to FUSE FIP image if available */
	io_result = fuse_fip_setup(fip_dev_con, boot_dev);

	assert(io_result == 0);

	return io_result;
}

#pragma weak ifc_nor_io_setup
#pragma weak ifc_nand_io_setup
#pragma weak qspi_io_setup
#pragma weak emmc_io_setup
#pragma weak flexspi_nor_io_setup

int qspi_io_setup(void)
{
	ERROR("QSPI driver not present. Check your BUILD\n");

	/* Should never reach here */
	assert(0);
	return -1;
}

int emmc_io_setup(void)
{
	ERROR("EMMC driver not present. Check your BUILD\n");

	/* Should never reach here */
	assert(0);
	return -1;
}

int emmc_sdhc2_io_setup(void)
{
	ERROR("EMMC driver not present. Check your BUILD\n");

	/* Should never reach here */
	assert(0);
	return -1;
}

int ifc_nor_io_setup(void)
{
	ERROR("NOR driver not present. Check your BUILD\n");

	/* Should never reach here */
	assert(0);
	return -1;
}

int ifc_nand_io_setup(void)
{
	ERROR("NAND driver not present. Check your BUILD\n");

	/* Should never reach here */
	assert(0);
	return -1;
}

int flexspi_nor_io_setup(void)
{
	ERROR("FlexSPI NOR driver not present. Check your BUILD\n");

	/* Should never reach here */
	assert(0);
	return -1;
}

static int (* const ls_io_setup_table[])(void) = {
	[BOOT_DEVICE_IFC_NOR] = ifc_nor_io_setup,
	[BOOT_DEVICE_IFC_NAND] = ifc_nand_io_setup,
	[BOOT_DEVICE_QSPI] = qspi_io_setup,
	[BOOT_DEVICE_EMMC] = emmc_io_setup,
	[BOOT_DEVICE_SDHC2_EMMC] = emmc_sdhc2_io_setup,
	[BOOT_DEVICE_FLEXSPI_NOR] = flexspi_nor_io_setup,
	[BOOT_DEVICE_FLEXSPI_NAND] = flexspi_nor_io_setup,
};


int plat_io_setup(void)
{
	int (*io_setup)(void);
	unsigned int boot_dev = BOOT_DEVICE_NONE;
	int ret;

	boot_dev = get_boot_dev();
	if (boot_dev == BOOT_DEVICE_NONE) {
		ERROR("Boot Device detection failed, Check RCW_SRC\n");
		return -EINVAL;
	}

	io_setup = ls_io_setup_table[boot_dev];
	ret = io_setup();
	if (ret)
		return ret;

	ret = ls_io_fip_setup(boot_dev);
	if (ret)
		return ret;

	return 0;
}

int plat_get_fuse_image_source(
	unsigned int image_id __unused,
	uintptr_t *dev_handle __unused,
	uintptr_t *image_spec __unused)
{
	/* By default do not try an alternative */
	return -ENOENT;
}

int plat_get_alt_image_source(
	unsigned int image_id __unused,
	uintptr_t *dev_handle __unused,
	uintptr_t *image_spec __unused)
{
	/* By default do not try an alternative */
	return -ENOENT;
}

/* Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result;
	const struct plat_io_policy *policy;

	if (image_id < ARRAY_SIZE(policies)) {

		policy = &policies[image_id];
		result = policy->check(policy->image_spec);
		if (result == 0) {
			*image_spec = policy->image_spec;
			*dev_handle = *(policy->dev_handle);
		}
	} else {
		VERBOSE("Trying alternative IO\n");
		result = plat_get_alt_image_source(image_id, dev_handle,
						       image_spec);
	}

	if (result != 0) {
		VERBOSE("Trying FUSE IO\n");
		result = plat_get_fuse_image_source(image_id, dev_handle,
						       image_spec);
	}

	return result;
}

/*
 * See if a Firmware Image Package is available,
 * by checking if TOC is valid or not.
 */
int ls_io_is_toc_valid(void)
{
	int result;

	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);

	return (result == 0);
}
