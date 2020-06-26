/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/object_pool.h>
#include <libfdt.h>

/* We currently use FW, TB_FW, SOC_FW, TOS_FW, NS_fw and HW configs  */
#define MAX_DTB_INFO	U(6)

static struct dyn_cfg_dtb_info_t dtb_infos[MAX_DTB_INFO];
static OBJECT_POOL_ARRAY(dtb_info_pool, dtb_infos);

/*
 * This function is used to alloc memory for fw config information from
 * global pool and set fw configuration information.
 * Specifically used by BL1 to set fw_config information in global array
 */
void set_fw_config_info(uintptr_t config_addr, uint32_t config_max_size)
{
	struct dyn_cfg_dtb_info_t *dtb_info;

	dtb_info = pool_alloc(&dtb_info_pool);
	dtb_info->config_addr = config_addr;
	dtb_info->config_max_size = config_max_size;
	dtb_info->config_id = FW_CONFIG_ID;
}

struct dyn_cfg_dtb_info_t *dyn_cfg_dtb_info_getter(unsigned int config_id)
{
	unsigned int index;
	struct dyn_cfg_dtb_info_t *info;

	/* Positions index to the proper config-id */
	for (index = 0; index < MAX_DTB_INFO; index++) {
		if (dtb_infos[index].config_id == config_id) {
			info = &dtb_infos[index];
			break;
		}
	}

	if (index == MAX_DTB_INFO) {
		WARN("FCONF: Invalid config id %u\n", config_id);
		info = NULL;
	}

	return info;
}

int fconf_populate_dtb_registry(uintptr_t config)
{
	int rc;
	int node, child;
	struct dyn_cfg_dtb_info_t *dtb_info;

	/* As libfdt use void *, we can't avoid this cast */
	const void *dtb = (void *)config;

	/*
	 * Compile time assert if FW_CONFIG_ID is 0 which is more
	 * unlikely as 0 is a valid image id for FIP as per the current
	 * code but still to avoid code breakage in case of unlikely
	 * event when image ids gets changed.
	 */
	CASSERT(FW_CONFIG_ID != 0, assert_invalid_fw_config_id);

	/*
	 * In case of BL1, fw_config dtb information is already
	 * populated in global dtb_infos array by 'set_fw_config_info'
	 * function, Below check is present to avoid re-population of
	 * fw_config information.
	 *
	 * Other BLs, satisfy below check and populate fw_config information
	 * in global dtb_infos array.
	 */
	if (dtb_infos[0].config_id == 0) {
		dtb_info = pool_alloc(&dtb_info_pool);
		dtb_info->config_addr = config;
		dtb_info->config_max_size = fdt_totalsize(dtb);
		dtb_info->config_id = FW_CONFIG_ID;
	}

	/* Find the node offset point to "fconf,dyn_cfg-dtb_registry" compatible property */
	const char *compatible_str = "fconf,dyn_cfg-dtb_registry";
	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	fdt_for_each_subnode(child, dtb, node) {
		uint32_t val32;
		uint64_t val64;

		dtb_info = pool_alloc(&dtb_info_pool);

		/* Read configuration dtb information */
		rc = fdt_read_uint64(dtb, child, "load-address", &val64);
		if (rc < 0) {
			ERROR("FCONF: Incomplete configuration property in dtb-registry.\n");
			return rc;
		}
		dtb_info->config_addr = (uintptr_t)val64;

		rc = fdt_read_uint32(dtb, child, "max-size", &val32);
		if (rc < 0) {
			ERROR("FCONF: Incomplete configuration property in dtb-registry.\n");
			return rc;
		}
		dtb_info->config_max_size = val32;

		rc = fdt_read_uint32(dtb, child, "id", &val32);
		if (rc < 0) {
			ERROR("FCONF: Incomplete configuration property in dtb-registry.\n");
			return rc;
		}
		dtb_info->config_id = val32;

		VERBOSE("FCONF: dyn_cfg.dtb_registry cell found with:\n");
		VERBOSE("\tload-address = %lx\n", dtb_info->config_addr);
		VERBOSE("\tmax-size = 0x%zx\n", dtb_info->config_max_size);
		VERBOSE("\tconfig-id = %u\n", dtb_info->config_id);
	}

	if ((child < 0) && (child != -FDT_ERR_NOTFOUND)) {
		ERROR("%d: fdt_for_each_subnode(): %d\n", __LINE__, node);
		return child;
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(FW_CONFIG, dyn_cfg, fconf_populate_dtb_registry);
