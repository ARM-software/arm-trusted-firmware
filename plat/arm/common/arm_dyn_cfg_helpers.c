/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <desc_image_load.h>
#include <fdt_wrappers.h>
#include <libfdt.h>
#include <plat_arm.h>

/*******************************************************************************
 * Helper to read the `hw_config` property in config DTB. This function
 * expects the following properties to be present in the config DTB.
 *	name : hw_config_addr		size : 2 cells
 *	name : hw_config_max_size	size : 1 cell
 *
 * Arguments:
 *	void *dtb		 - pointer to the TB_FW_CONFIG in memory
 *	int node		 - The node offset to appropriate node in the
 *					 DTB.
 *	uint64_t *hw_config_addr - Returns the `hw_config` load address if read
 *					 is successful.
 *	uint32_t *hw_config_size - Returns the `hw_config` size if read is
 *					 successful.
 *
 * Returns 0 on success and -1 on error.
 ******************************************************************************/
int arm_dyn_get_hwconfig_info(void *dtb, int node,
		uint64_t *hw_config_addr, uint32_t *hw_config_size)
{
	int err;

	assert(dtb != NULL);
	assert(hw_config_addr != NULL);
	assert(hw_config_size != NULL);

	/* Check if the pointer to DT is correct */
	assert(fdt_check_header(dtb) == 0);

	/* Assert the node offset point to "arm,tb_fw" compatible property */
	assert(node == fdt_node_offset_by_compatible(dtb, -1, "arm,tb_fw"));

	err = fdtw_read_cells(dtb, node, "hw_config_addr", 2,
				(void *) hw_config_addr);
	if (err < 0) {
		WARN("Read cell failed for hw_config_addr\n");
		return -1;
	}

	err = fdtw_read_cells(dtb, node, "hw_config_max_size", 1,
				(void *) hw_config_size);
	if (err < 0) {
		WARN("Read cell failed for hw_config_max_size\n");
		return -1;
	}

	VERBOSE("Dyn cfg: Read hw_config address from TB_FW_CONFIG 0x%p %p\n",
				hw_config_addr, hw_config_size);

	return 0;
}

/*******************************************************************************
 * Validate the tb_fw_config is a valid DTB file and returns the node offset
 * to "arm,tb_fw" property.
 * Arguments:
 *	void *dtb - pointer to the TB_FW_CONFIG in memory
 *	int *node - Returns the node offset to "arm,tb_fw" property if found.
 *
 * Returns 0 on success and -1 on error.
 ******************************************************************************/
int arm_dyn_tb_fw_cfg_init(void *dtb, int *node)
{
	assert(dtb != NULL);
	assert(node != NULL);

	/* Check if the pointer to DT is correct */
	if (fdt_check_header(dtb) != 0) {
		WARN("Invalid DTB file passed as TB_FW_CONFIG\n");
		return -1;
	}

	/* Assert the node offset point to "arm,tb_fw" compatible property */
	*node = fdt_node_offset_by_compatible(dtb, -1, "arm,tb_fw");
	if (*node < 0) {
		WARN("The compatible property `arm,tb_fw` not found in the config\n");
		return -1;
	}

	VERBOSE("Dyn cfg: Found \"arm,tb_fw\" in the config\n");
	return 0;
}
