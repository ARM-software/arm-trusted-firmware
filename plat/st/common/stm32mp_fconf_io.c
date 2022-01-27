/*
 * Copyright (c) 2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/io/io_storage.h>
#include <drivers/mmc.h>
#include <lib/fconf/fconf.h>
#include <lib/object_pool.h>
#include <libfdt.h>
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>
#include <stm32mp_efi.h>
#include <stm32mp_fconf_getter.h>
#include <stm32mp_io_storage.h>

#if STM32MP_SDMMC || STM32MP_EMMC
static io_block_spec_t gpt_block_spec = {
	.offset = 0U,
	.length = 34U * MMC_BLOCK_SIZE, /* Size of GPT table */
};
#endif

#if (STM32MP_SDMMC || STM32MP_EMMC) && PSA_FWU_SUPPORT
io_block_spec_t metadata_block_spec = {
	.offset = 0,    /* To be filled at runtime */
	.length = 0,    /* To be filled at runtime */
};
#endif /* (STM32MP_SDMMC || STM32MP_EMMC) && PSA_FWU_SUPPORT */

/* By default, STM32 platforms load images from the FIP */
struct plat_io_policy policies[MAX_NUMBER_IDS] = {
	[FIP_IMAGE_ID] = {
		.dev_handle = &storage_dev_handle,
		.image_spec = (uintptr_t)&image_block_spec,
		.img_type_guid = STM32MP_FIP_GUID,
		.check = open_storage
	},
#if STM32MP_SDMMC || STM32MP_EMMC
	[GPT_IMAGE_ID] = {
		.dev_handle = &storage_dev_handle,
		.image_spec = (uintptr_t)&gpt_block_spec,
		.img_type_guid = NULL_GUID,
		.check = open_storage
	},
#endif
#if (STM32MP_SDMMC || STM32MP_EMMC) && PSA_FWU_SUPPORT
	[FWU_METADATA_IMAGE_ID] = {
		.dev_handle = &storage_dev_handle,
		.image_spec = (uintptr_t)&metadata_block_spec,
		.img_type_guid = NULL_GUID,
		.check = open_storage
	},
	[BKUP_FWU_METADATA_IMAGE_ID] = {
		.dev_handle = &storage_dev_handle,
		.image_spec = (uintptr_t)&metadata_block_spec,
		.img_type_guid = NULL_GUID,
		.check = open_storage
	},
#endif /* (STM32MP_SDMMC || STM32MP_EMMC) && PSA_FWU_SUPPORT */
};

#define FCONF_ST_IO_UUID_NUMBER	U(8)

static io_uuid_spec_t fconf_stm32mp_uuids[FCONF_ST_IO_UUID_NUMBER];
static OBJECT_POOL_ARRAY(fconf_stm32mp_uuids_pool, fconf_stm32mp_uuids);

struct policies_load_info {
	unsigned int image_id;
	const char *name;
};

/* image id to property name table */
static const struct policies_load_info load_info[FCONF_ST_IO_UUID_NUMBER] = {
	{FW_CONFIG_ID, "fw_cfg_uuid"},
	{BL32_IMAGE_ID, "bl32_uuid"},
	{BL32_EXTRA1_IMAGE_ID, "bl32_extra1_uuid"},
	{BL32_EXTRA2_IMAGE_ID, "bl32_extra2_uuid"},
	{BL33_IMAGE_ID, "bl33_uuid"},
	{HW_CONFIG_ID, "hw_cfg_uuid"},
	{TOS_FW_CONFIG_ID, "tos_fw_cfg_uuid"},
	{NT_FW_CONFIG_ID, "nt_fw_cfg_uuid"},
};

int fconf_populate_stm32mp_io_policies(uintptr_t config)
{
	int node;
	unsigned int i;

	/* As libfdt uses void *, we can't avoid this cast */
	const void *dtb = (void *)config;

	/* Assert the node offset point to "st,io-fip-handle" compatible property */
	const char *compatible_str = "st,io-fip-handle";

	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	/* Locate the uuid cells and read the value for all the load info uuid */
	for (i = 0U; i < FCONF_ST_IO_UUID_NUMBER; i++) {
		union uuid_helper_t uuid_helper;
		io_uuid_spec_t *uuid_ptr;
		int err;

		uuid_ptr = pool_alloc(&fconf_stm32mp_uuids_pool);
		err = fdtw_read_uuid(dtb, node, load_info[i].name, 16,
				     (uint8_t *)&uuid_helper);
		if (err < 0) {
			WARN("FCONF: Read cell failed for %s\n", load_info[i].name);
			return err;
		}

		VERBOSE("FCONF: stm32mp-io_policies.%s cell found with value = "
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
			load_info[i].name,
			uuid_helper.uuid_struct.time_low[0], uuid_helper.uuid_struct.time_low[1],
			uuid_helper.uuid_struct.time_low[2], uuid_helper.uuid_struct.time_low[3],
			uuid_helper.uuid_struct.time_mid[0], uuid_helper.uuid_struct.time_mid[1],
			uuid_helper.uuid_struct.time_hi_and_version[0],
			uuid_helper.uuid_struct.time_hi_and_version[1],
			uuid_helper.uuid_struct.clock_seq_hi_and_reserved,
			uuid_helper.uuid_struct.clock_seq_low,
			uuid_helper.uuid_struct.node[0], uuid_helper.uuid_struct.node[1],
			uuid_helper.uuid_struct.node[2], uuid_helper.uuid_struct.node[3],
			uuid_helper.uuid_struct.node[4], uuid_helper.uuid_struct.node[5]);

		uuid_ptr->uuid = uuid_helper.uuid_struct;
		policies[load_info[i].image_id].image_spec = (uintptr_t)uuid_ptr;
		policies[load_info[i].image_id].dev_handle = &fip_dev_handle;
		policies[load_info[i].image_id].check = open_fip;
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(TB_FW, stm32mp_io, fconf_populate_stm32mp_io_policies);
