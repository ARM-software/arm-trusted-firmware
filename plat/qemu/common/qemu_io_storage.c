/*
 * Copyright (c) 2015-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/uuid.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_encrypted.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>
#include <drivers/io/io_semihosting.h>
#include <drivers/io/io_storage.h>
#include <lib/semihosting.h>
#include <tools_share/firmware_image_package.h>

#include "qemu_private.h"

/* Semihosting filenames */
#define BL2_IMAGE_NAME			"bl2.bin"
#define BL31_IMAGE_NAME			"bl31.bin"
#define BL32_IMAGE_NAME			"bl32.bin"
#define TB_FW_CONFIG_NAME		"tb_fw_config.dtb"
#define TOS_FW_CONFIG_NAME		"tos_fw_config.dtb"
#define BL32_EXTRA1_IMAGE_NAME		"bl32_extra1.bin"
#define BL32_EXTRA2_IMAGE_NAME		"bl32_extra2.bin"
#define BL33_IMAGE_NAME			"bl33.bin"

#if TRUSTED_BOARD_BOOT
#define TRUSTED_BOOT_FW_CERT_NAME	"tb_fw.crt"
#define TRUSTED_KEY_CERT_NAME		"trusted_key.crt"
#define SOC_FW_KEY_CERT_NAME		"soc_fw_key.crt"
#define TOS_FW_KEY_CERT_NAME		"tos_fw_key.crt"
#define NT_FW_KEY_CERT_NAME		"nt_fw_key.crt"
#define SOC_FW_CONTENT_CERT_NAME	"soc_fw_content.crt"
#define TOS_FW_CONTENT_CERT_NAME	"tos_fw_content.crt"
#define NT_FW_CONTENT_CERT_NAME		"nt_fw_content.crt"
#endif /* TRUSTED_BOARD_BOOT */



/* IO devices */
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;
static const io_dev_connector_t *sh_dev_con;
static uintptr_t sh_dev_handle;
#ifndef DECRYPTION_SUPPORT_none
static const io_dev_connector_t *enc_dev_con;
static uintptr_t enc_dev_handle;
#endif

static const io_block_spec_t fip_block_spec = {
	.offset = PLAT_QEMU_FIP_BASE,
	.length = PLAT_QEMU_FIP_MAX_SIZE
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

static const io_uuid_spec_t bl32_extra1_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA1,
};

static const io_uuid_spec_t bl32_extra2_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA2,
};

static const io_uuid_spec_t tb_fw_config_uuid_spec = {
	.uuid = UUID_TB_FW_CONFIG,
};

static const io_uuid_spec_t tos_fw_config_uuid_spec = {
	.uuid = UUID_TOS_FW_CONFIG,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t tb_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FW_CERT,
};

static const io_uuid_spec_t trusted_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
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

static const io_file_spec_t sh_file_spec[] = {
	[BL2_IMAGE_ID] = {
		.path = BL2_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL31_IMAGE_ID] = {
		.path = BL31_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL32_IMAGE_ID] = {
		.path = BL32_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		.path = BL32_EXTRA1_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		.path = BL32_EXTRA2_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
	[TB_FW_CONFIG_ID] = {
		.path = TB_FW_CONFIG_NAME,
		.mode = FOPEN_MODE_RB
	},
	[TOS_FW_CONFIG_ID] = {
		.path = TOS_FW_CONFIG_NAME,
		.mode = FOPEN_MODE_RB
	},
	[BL33_IMAGE_ID] = {
		.path = BL33_IMAGE_NAME,
		.mode = FOPEN_MODE_RB
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		.path = TRUSTED_BOOT_FW_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[TRUSTED_KEY_CERT_ID] = {
		.path = TRUSTED_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[SOC_FW_KEY_CERT_ID] = {
		.path = SOC_FW_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		.path = TOS_FW_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		.path = NT_FW_KEY_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		.path = SOC_FW_CONTENT_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		.path = TOS_FW_CONTENT_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		.path = NT_FW_CONTENT_CERT_NAME,
		.mode = FOPEN_MODE_RB
	},
#endif /* TRUSTED_BOARD_BOOT */
};

static int open_fip(const uintptr_t spec);
static int open_memmap(const uintptr_t spec);
#ifndef DECRYPTION_SUPPORT_none
static int open_enc_fip(const uintptr_t spec);
#endif

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* By default, ARM platforms load images from the FIP */
static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_memmap
	},
	[ENC_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)NULL,
		open_fip
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		open_fip
	},
#if ENCRYPT_BL31 && !defined(DECRYPTION_SUPPORT_none)
	[BL31_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		open_enc_fip
	},
#else
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		open_fip
	},
#endif
#if ENCRYPT_BL32 && !defined(DECRYPTION_SUPPORT_none)
	[BL32_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		open_enc_fip
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		open_enc_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		open_enc_fip
	},
#else
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
#endif
	[TB_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tb_fw_config_uuid_spec,
		open_fip
	},
	[TOS_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_config_uuid_spec,
		open_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
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

#if defined(SPD_spmd)
static struct sp_pkg {
	struct plat_io_policy policy;
	io_file_spec_t sh_file_spec;
	uint8_t uuid[UUID_BYTES_LENGTH];
	char path[80];
} sp_pkgs[MAX_SP_IDS];
static unsigned int sp_pkg_count;

int qemu_io_register_sp_pkg(const char *name, const char *uuid,
			    uintptr_t load_addr)
{
	struct sp_pkg *pkg;
	bl_mem_params_node_t *mem_params;

	if (sp_pkg_count == MAX_SP_IDS) {
		INFO("Reached Max number of SPs\n");
		return -1;
	}
	mem_params = get_bl_mem_params_node(SP_PKG1_ID + sp_pkg_count);
	if (mem_params == NULL) {
		ERROR("Can't find SP_PKG ID %u (SP_PKG%u_ID)\n",
		      SP_PKG1_ID + sp_pkg_count, sp_pkg_count);
		return -1;
	}
	pkg = sp_pkgs + sp_pkg_count;

	if (read_uuid(pkg->uuid, (char *)uuid)) {
		return -1;
	}

	strlcpy(pkg->path, name, sizeof(pkg->path));
	strlcat(pkg->path, ".pkg", sizeof(pkg->path));

	pkg->policy.dev_handle = &fip_dev_handle;
	pkg->policy.image_spec = (uintptr_t)&pkg->uuid;
	pkg->policy.check = open_fip;
	pkg->sh_file_spec.path = pkg->path;
	pkg->sh_file_spec.mode = FOPEN_MODE_RB;

	mem_params->image_info.image_base = load_addr;
	mem_params->image_info.image_max_size = SZ_4M;
	mem_params->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;

	sp_pkg_count++;

	return 0;
}
#endif /*SPD_spmd*/

static const io_file_spec_t *get_io_file_spec(unsigned int image_id)
{
#if defined(SPD_spmd)
	if (image_id >= SP_PKG1_ID && image_id <= SP_PKG8_ID) {
		return &sp_pkgs[image_id - SP_PKG1_ID].sh_file_spec;
	}
#endif

	assert(image_id < ARRAY_SIZE(sh_file_spec));
	return &sh_file_spec[image_id];
}

static const struct plat_io_policy *get_io_policy(unsigned int image_id)
{
#if defined(SPD_spmd)
	if (image_id >= SP_PKG1_ID && image_id <= SP_PKG8_ID) {
		return &sp_pkgs[image_id - SP_PKG1_ID].policy;
	}
#endif

	assert(image_id < ARRAY_SIZE(policies));
	return &policies[image_id];
}

static int open_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0 && spec != (uintptr_t)NULL) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

#ifndef DECRYPTION_SUPPORT_none
static int open_enc_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if an encrypted FIP is available */
	result = io_dev_init(enc_dev_handle, (uintptr_t)ENC_IMAGE_ID);
	if (result == 0) {
		result = io_open(enc_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using encrypted FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}
#endif

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

static int open_semihosting(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if the file exists on semi-hosting.*/
	result = io_dev_init(sh_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(sh_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using Semi-hosting IO\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

void plat_qemu_io_setup(void)
{
	int io_result;

	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == 0);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	assert(io_result == 0);

	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&memmap_dev_handle);
	assert(io_result == 0);

#ifndef DECRYPTION_SUPPORT_none
	io_result = register_io_dev_enc(&enc_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(enc_dev_con, (uintptr_t)NULL,
				&enc_dev_handle);
	assert(io_result == 0);
#endif

	/* Register the additional IO devices on this platform */
	io_result = register_io_dev_sh(&sh_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(sh_dev_con, (uintptr_t)NULL, &sh_dev_handle);
	assert(io_result == 0);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}

static int get_alt_image_source(unsigned int image_id, uintptr_t *dev_handle,
				  uintptr_t *image_spec)
{
	const io_file_spec_t *spec = get_io_file_spec(image_id);
	int result;

	result = open_semihosting((const uintptr_t)spec);
	if (result == 0) {
		*dev_handle = sh_dev_handle;
		*image_spec = (uintptr_t)spec;
	}

	return result;
}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	const struct plat_io_policy *policy = get_io_policy(image_id);
	int result;

	result = policy->check(policy->image_spec);
	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	} else {
		VERBOSE("Trying alternative IO\n");
		result = get_alt_image_source(image_id, dev_handle, image_spec);
	}

	return result;
}
