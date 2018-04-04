/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_dyn_cfg_helpers.h>
#include <assert.h>
#include <desc_image_load.h>
#include <fdt_wrappers.h>
#include <libfdt.h>
#include <plat_arm.h>


typedef struct config_load_info_prop {
	unsigned int config_id;
	const char *config_addr;
	const char *config_max_size;
} config_load_info_prop_t;

static const config_load_info_prop_t prop_names[] = {
	{HW_CONFIG_ID, "hw_config_addr", "hw_config_max_size"},
	{SOC_FW_CONFIG_ID, "soc_fw_config_addr", "soc_fw_config_max_size"},
	{TOS_FW_CONFIG_ID, "tos_fw_config_addr", "tos_fw_config_max_size"},
	{NT_FW_CONFIG_ID, "nt_fw_config_addr", "nt_fw_config_max_size"}
};

/*******************************************************************************
 * Helper to read the load information corresponding to the `config_id` in
 * TB_FW_CONFIG. This function expects the following properties to be defined :
 *	<config>_addr		size : 2 cells
 *	<config>_max_size	size : 1 cell
 *
 * Arguments:
 *	void *dtb		 - pointer to the TB_FW_CONFIG in memory
 *	int node		 - The node offset to appropriate node in the
 *					 DTB.
 *	unsigned int config_id	 - The configuration id
 *	uint64_t *config_addr	 - Returns the `config` load address if read
 *					 is successful.
 *	uint32_t *config_size	 - Returns the `config` size if read is
 *					 successful.
 *
 * Returns 0 on success and -1 on error.
 ******************************************************************************/
int arm_dyn_get_config_load_info(void *dtb, int node, unsigned int config_id,
		uint64_t *config_addr, uint32_t *config_size)
{
	int err;
	unsigned int i;

	assert(dtb != NULL);
	assert(config_addr != NULL);
	assert(config_size != NULL);

	for (i = 0; i < ARRAY_SIZE(prop_names); i++) {
		if (prop_names[i].config_id == config_id)
			break;
	}

	if (i == ARRAY_SIZE(prop_names)) {
		WARN("Invalid config id %d\n", config_id);
		return -1;
	}

	/* Check if the pointer to DT is correct */
	assert(fdt_check_header(dtb) == 0);

	/* Assert the node offset point to "arm,tb_fw" compatible property */
	assert(node == fdt_node_offset_by_compatible(dtb, -1, "arm,tb_fw"));

	err = fdtw_read_cells(dtb, node, prop_names[i].config_addr, 2,
				(void *) config_addr);
	if (err < 0) {
		WARN("Read cell failed for %s\n", prop_names[i].config_addr);
		return -1;
	}

	err = fdtw_read_cells(dtb, node, prop_names[i].config_max_size, 1,
				(void *) config_size);
	if (err < 0) {
		WARN("Read cell failed for %s\n", prop_names[i].config_max_size);
		return -1;
	}

	VERBOSE("Dyn cfg: Read config_id %d load info from TB_FW_CONFIG 0x%llx 0x%x\n",
				config_id, (unsigned long long)*config_addr, *config_size);

	return 0;
}

/*******************************************************************************
 * Helper to read the `disable_auth` property in config DTB. This function
 * expects the following properties to be present in the config DTB.
 *	name : disable_auth		size : 1 cell
 *
 * Arguments:
 *	void *dtb		 - pointer to the TB_FW_CONFIG in memory
 *	int node		 - The node offset to appropriate node in the
 *				   DTB.
 *	uint64_t *disable_auth	 - The value of `disable_auth` property on
 *				   successful read. Must be 0 or 1.
 *
 * Returns 0 on success and -1 on error.
 ******************************************************************************/
int arm_dyn_get_disable_auth(void *dtb, int node, uint32_t *disable_auth)
{
	int err;

	assert(dtb != NULL);
	assert(disable_auth != NULL);

	/* Check if the pointer to DT is correct */
	assert(fdt_check_header(dtb) == 0);

	/* Assert the node offset point to "arm,tb_fw" compatible property */
	assert(node == fdt_node_offset_by_compatible(dtb, -1, "arm,tb_fw"));

	/* Locate the disable_auth cell and read the value */
	err = fdtw_read_cells(dtb, node, "disable_auth", 1, disable_auth);
	if (err < 0) {
		WARN("Read cell failed for `disable_auth`\n");
		return -1;
	}

	/* Check if the value is boolean */
	if ((*disable_auth != 0U) && (*disable_auth != 1U)) {
		WARN("Invalid value for `disable_auth` cell %d\n", *disable_auth);
		return -1;
	}

	VERBOSE("Dyn cfg: `disable_auth` cell found with value = %d\n",
					*disable_auth);
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
