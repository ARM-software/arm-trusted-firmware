/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <libfdt.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <sgi_variant.h>

/*******************************************************************************
 * This function inserts Platform information via device tree nodes as,
 * system-id {
 *    platform-id = <0>;
 *    config-id = <0>;
 * }
 ******************************************************************************/
static int plat_sgi_append_config_node(void)
{
	bl_mem_params_node_t *mem_params;
	void *fdt;
	int nodeoffset, err;
	unsigned int platid = 0, platcfg = 0;

	mem_params = get_bl_mem_params_node(NT_FW_CONFIG_ID);
	if (mem_params == NULL) {
		ERROR("NT_FW CONFIG base address is NULL");
		return -1;
	}

	fdt = (void *)(mem_params->image_info.image_base);

	/* Check the validity of the fdt */
	if (fdt_check_header(fdt) != 0) {
		ERROR("Invalid NT_FW_CONFIG DTB passed\n");
		return -1;
	}

	nodeoffset = fdt_subnode_offset(fdt, 0, "system-id");
	if (nodeoffset < 0) {
		ERROR("Failed to get system-id node offset\n");
		return -1;
	}

	platid = plat_arm_sgi_get_platform_id();
	err = fdt_setprop_u32(fdt, nodeoffset, "platform-id", platid);
	if (err < 0) {
		ERROR("Failed to set platform-id\n");
		return -1;
	}

	platcfg = plat_arm_sgi_get_config_id();
	err = fdt_setprop_u32(fdt, nodeoffset, "config-id", platcfg);
	if (err < 0) {
		ERROR("Failed to set config-id\n");
		return -1;
	}

	platcfg = plat_arm_sgi_get_multi_chip_mode();
	err = fdt_setprop_u32(fdt, nodeoffset, "multi-chip-mode", platcfg);
	if (err < 0) {
		ERROR("Failed to set multi-chip-mode\n");
		return -1;
	}

	flush_dcache_range((uintptr_t)fdt, mem_params->image_info.image_size);

	return 0;
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	int ret;

	ret = plat_sgi_append_config_node();
	if (ret != 0)
		panic();

	return arm_get_next_bl_params();
}

