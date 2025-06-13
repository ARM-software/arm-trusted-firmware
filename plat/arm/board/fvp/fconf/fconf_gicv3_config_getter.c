/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/arm/gicv3.h>

#include <fconf_hw_config_getter.h>

struct gicv3_config_t gicv3_config;

int fconf_populate_gicv3_config(uintptr_t config)
{
	int err;
	int node;
	uintptr_t addr;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)config;

	/*
	 * Find the offset of the node containing "arm,gic-v3" compatible property.
	 * Populating fconf strucutures dynamically is not supported for legacy
	 * systems which use GICv2 IP. Simply skip extracting GIC properties.
	 */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,gic-v3");
	if (node < 0) {
		WARN("FCONF: Unable to locate node with arm,gic-v3 compatible property\n");
		return 0;
	}
	/* The GICv3 DT binding holds at least two address/size pairs,
	 * the first describing the distributor, the second the redistributors.
	 * See: bindings/interrupt-controller/arm,gic-v3.yaml
	 */
	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 0, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICD reg property of GIC node\n");
		return err;
	}
	gicv3_config.gicd_base = addr;

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 1, &addr, NULL);
	if (err < 0) {
		ERROR("FCONF: Failed to read GICR reg property of GIC node\n");
	} else {
		gicv3_config.gicr_base = addr;
	}

	return err;
}
FCONF_REGISTER_POPULATOR(HW_CONFIG, gicv3_config, fconf_populate_gicv3_config);
