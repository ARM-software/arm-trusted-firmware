/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/fconf/fconf.h>
#include <libfdt.h>
#include <plat/common/platform.h>
#include <platform_def.h>

struct fconf_dtb_info_t fconf_dtb_info;

void fconf_load_config(void)
{
	int err;
	/* fconf FW_CONFIG and TB_FW_CONFIG are currently the same DTB */
	image_info_t arm_tb_fw_info = {
		.h.type = (uint8_t)PARAM_IMAGE_BINARY,
		.h.version = (uint8_t)VERSION_2,
		.h.size = (uint16_t)sizeof(image_info_t),
		.h.attr = 0,
		.image_base = ARM_TB_FW_CONFIG_BASE,
		.image_max_size = (uint32_t)
				(ARM_TB_FW_CONFIG_LIMIT - ARM_TB_FW_CONFIG_BASE)
	};

	VERBOSE("FCONF: Loading FW_CONFIG\n");
	err = load_auth_image(TB_FW_CONFIG_ID, &arm_tb_fw_info);
	if (err != 0) {
		/* Return if FW_CONFIG is not loaded */
		VERBOSE("Failed to load FW_CONFIG\n");
		return;
	}

	/* At this point we know that a DTB is indeed available */
	fconf_dtb_info.base_addr = arm_tb_fw_info.image_base;
	fconf_dtb_info.size = (size_t)arm_tb_fw_info.image_size;

#if !BL2_AT_EL3
	image_desc_t *desc;

	/* The BL2 ep_info arg0 is modified to point to FW_CONFIG */
	desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(desc != NULL);
	desc->ep_info.args.arg0 = arm_tb_fw_info.image_base;
#endif

	INFO("FCONF: FW_CONFIG loaded at address = 0x%lx\n", arm_tb_fw_info.image_base);
}

void fconf_populate(uintptr_t config)
{
	assert(config != 0UL);

	/* Check if the pointer to DTB is correct */
	if (fdt_check_header((void *)config) != 0) {
		ERROR("FCONF: Invalid DTB file passed for FW_CONFIG\n");
		panic();
	}

	INFO("FCONF: Reading firmware configuration file from: 0x%lx\n", config);

	/* Go through all registered populate functions */
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_START__, start);
	IMPORT_SYM(struct fconf_populator *, __FCONF_POPULATOR_END__, end);
	const struct fconf_populator *populator;

	for (populator = start; populator != end; populator++) {
		assert((populator->info != NULL) && (populator->populate != NULL));

		INFO("FCONF: Reading firmware configuration information for: %s\n", populator->info);
		if (populator->populate(config) != 0) {
			/* TODO: handle property miss */
			panic();
		}
	}

	/* save local pointer to the config dtb */
	fconf_dtb_info.base_addr = config;
}
