/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <desc_image_load.h>
#include <libfdt.h>
#include <platform.h>

/*******************************************************************************
 * This function inserts Platform information via device tree nodes as,
 * system-id {
 *    platform-id = <0>;
 * }
 ******************************************************************************/
static int plat_sgi_append_config_node(void)
{
	bl_mem_params_node_t *mem_params;
	void *fdt;
	int nodeoffset, err;
	unsigned int platid = 0;
	char *platform_name;

	mem_params = get_bl_mem_params_node(HW_CONFIG_ID);
	if (mem_params == NULL) {
		ERROR("HW CONFIG base address is NULL");
		return -1;
	}

	fdt = (void *)(mem_params->image_info.image_base);

	/* Check the validity of the fdt */
	if (fdt_check_header(fdt) != 0) {
		ERROR("Invalid HW_CONFIG DTB passed\n");
		return -1;
	}

	platform_name = (char *)fdt_getprop(fdt, 0, "compatible", NULL);

	if (strcmp(platform_name, "arm,sgi575") == 0) {
		platid = mmio_read_32(SSC_VERSION);
	} else {
		WARN("Invalid platform \n");
		return -1;
	}

	/* Increase DTB blob by 512 byte */
	err = fdt_open_into(fdt, fdt, mem_params->image_info.image_size + 512);
	if (err < 0) {
		ERROR("Failed to open HW_CONFIG DTB\n");
		return -1;
	}

	/* Create "/system-id" node */
	nodeoffset = fdt_add_subnode(fdt, 0, "system-id");
	if (nodeoffset < 0) {
		ERROR("Failed to add node system-id\n");
		return -1;
	}

	err = fdt_setprop_u32(fdt, nodeoffset, "platform-id", platid);
	if (err < 0) {
		ERROR("Failed to add node platform-id\n");
		return -1;
	}
	return 0;
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	int ret;
	bl_params_t *next_bl_params;

	ret = plat_sgi_append_config_node();
	if (ret != 0)
		panic();

	next_bl_params = get_next_bl_params_from_mem_params_desc();
	populate_next_bl_params_config(next_bl_params);

	return next_bl_params;
}
