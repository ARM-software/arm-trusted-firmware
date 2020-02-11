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

/* We currently use TB_FW, SOC_FW, TOS_FW, NS_fw and HW configs  */
#define MAX_DTB_INFO	U(5)

static struct dyn_cfg_dtb_info_t dtb_infos[MAX_DTB_INFO];
static OBJECT_POOL_ARRAY(dtb_info_pool, dtb_infos);

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

	/* Find the node offset point to "arm,dyn_cfg-dtb_registry" compatible property */
	const char *compatible_str = "arm,dyn_cfg-dtb_registry";
	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	fdt_for_each_subnode(child, dtb, node) {
		dtb_info = pool_alloc(&dtb_info_pool);

		/* Read configuration dtb information */
		rc = fdtw_read_cells(dtb, child, "load-address", 2, &dtb_info->config_addr);
		if (rc < 0) {
			ERROR("FCONF: Incomplete configuration property in dtb-registry.\n");
			return rc;
		}

		rc = fdtw_read_cells(dtb, child, "max-size", 1, &dtb_info->config_max_size);
		if (rc < 0) {
			ERROR("FCONF: Incomplete configuration property in dtb-registry.\n");
			return rc;
		}

		rc = fdtw_read_cells(dtb, child, "id", 1, &dtb_info->config_id);
		if (rc < 0) {
			ERROR("FCONF: Incomplete configuration property in dtb-registry.\n");
			return rc;
		}

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

FCONF_REGISTER_POPULATOR(dyn_cfg, fconf_populate_dtb_registry);
