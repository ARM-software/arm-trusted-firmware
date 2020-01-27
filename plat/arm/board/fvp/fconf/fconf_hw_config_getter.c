/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <fconf_hw_config_getter.h>
#include <libfdt.h>
#include <plat/common/platform.h>

struct gicv3_config_t gicv3_config;

int fconf_populate_gicv3_config(uintptr_t config)
{
	int err;
	int node;
	int addr[20];

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
	/* Read the reg cell holding base address of GIC controller modules
	A sample reg cell array is shown here:
		reg = <0x0 0x2f000000 0 0x10000>,	// GICD
		      <0x0 0x2f100000 0 0x200000>,	// GICR
		      <0x0 0x2c000000 0 0x2000>,	// GICC
		      <0x0 0x2c010000 0 0x2000>,	// GICH
		      <0x0 0x2c02f000 0 0x2000>;	// GICV
	*/

	err = fdtw_read_array(hw_config_dtb, node, "reg", 20, &addr);
	if (err < 0) {
		ERROR("FCONF: Failed to read reg property of GIC node\n");
	}
	return err;
}


FCONF_REGISTER_POPULATOR(HW_CONFIG, gicv3_config, fconf_populate_gicv3_config);
