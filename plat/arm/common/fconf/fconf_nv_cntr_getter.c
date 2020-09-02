/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>

#include <libfdt.h>

#include <plat/arm/common/fconf_nv_cntr_getter.h>

/*******************************************************************************
 * fconf_populate_cot_descs() - Populate available nv-counters and update global
 *				structure.
 * @config[in]:	Pointer to the device tree blob in memory
 *
 * Return 0 on success or an error value otherwise.
 ******************************************************************************/
static int fconf_populate_nv_cntrs(uintptr_t config)
{
	int rc, node, child;
	uint32_t id;
	uintptr_t reg;

	/* As libfdt uses void *, we can't avoid this cast */
	const void *dtb = (void *)config;
	const char *compatible_str = "arm, non-volatile-counter";

	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in node\n",
			compatible_str);
		return node;
	}

	fdt_for_each_subnode(child, dtb, node) {

		rc = fdt_read_uint32(dtb, child, "id", &id);
		if (rc < 0) {
			ERROR("FCONF: Can't find %s property in node\n", "id");
			return rc;
		}

		assert(id < MAX_NV_CTR_IDS);

		rc = fdt_get_reg_props_by_index(dtb, child, 0, &reg, NULL);
		if (rc < 0) {
			ERROR("FCONF: Can't find %s property in node\n", "reg");
			return rc;
		}

		nv_cntr_base_addr[id] = reg;
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(TB_FW, nv_cntrs, fconf_populate_nv_cntrs);
