/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <libfdt.h>
#include <platform_def.h>
#include <tools_share/firmware_image_package.h>

#include "socfpga_dt.h"

static const void *fdt;
/*******************************************************************************
 * This function checks device tree file with its header.
 * Returns 0 on success and a negative FDT error code on failure.
 ******************************************************************************/
int socfpga_dt_open_and_check(uintptr_t dt_addr, char *compatible_str)
{
	int ret = 0;
	int node = 1;

	ret = fdt_check_header((void *)dt_addr);

	if (ret != 0) {
		ERROR("SOCFPGA: FDT Header invalid\n");
		return ret;
	}

	fdt = (const void *)dt_addr;

	/* As libfdt use void *, we can't avoid this cast */
	const void *dtb = (void *)dt_addr;

	/* Assert the node offset point to compatible property */
	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("SOCFPGA: Can't find `%s` compatible in dtb\n",
						compatible_str);
		return node;
	}

	NOTICE("SOCFPGA: Successfully open and check FDT\n");

	return ret;
}

int socfpga_dt_populate_gicv3_config(uintptr_t dt_addr, gicv3_driver_data_t *plat_driver_data)
{
	int err;
	int node;
	uintptr_t addr;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)dt_addr;
	/*
	 * Find the offset of the node containing "arm,gic-v3" compatible property.
	 * Populating fconf strucutures dynamically is not supported for legacy
	 * systems which use GICv2 IP. Simply skip extracting GIC properties.
	 */
	node = fdt_node_offset_by_compatible(hw_config_dtb, -1, "arm,gic-v3");
	if (node < 0) {
		ERROR("SOCFPGA: Unable to locate node with arm,gic-v3 compatible property\n");
		return 0;
	}
	/* The GICv3 DT binding holds at least two address/size pairs,
	 * the first describing the distributor, the second the redistributors.
	 * See: bindings/interrupt-controller/arm,gic-v3.yaml
	 */
	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 0, &addr, NULL);
	if (err < 0) {
		ERROR("SOCFPGA: Failed to read GICD reg property of GIC node\n");
	} else {
		plat_driver_data->gicd_base = addr;
	}

	err = fdt_get_reg_props_by_index(hw_config_dtb, node, 1, &addr, NULL);
	if (err < 0) {
		ERROR("SOCFPGA: Failed to read GICR reg property of GIC node\n");
	} else {
		plat_driver_data->gicr_base = addr;
	}
	return err;
}

int socfpga_dt_populate_dram_layout(uintptr_t dt_addr)
{
	int node;
	uintptr_t addr;
	size_t size;

	/* Necessary to work with libfdt APIs */
	const void *hw_config_dtb = (const void *)dt_addr;

	/* Find 'memory' node */
	node = fdt_node_offset_by_prop_value(hw_config_dtb, -1, "device_type",
					     "memory", sizeof("memory"));
	if (node < 0) {
		NOTICE("SOCFPGA: Unable to locate 'memory' node\n");
		return node;
	}

	int err = fdt_get_reg_props_by_index(
			hw_config_dtb, node, 0,
			&addr, (size_t *)&size);

	NOTICE("SOCFPGA: Mem base 0x%lx, Mem size 0x%lx\n", addr, size);
	if (err < 0) {
		ERROR("SOCFPGA: Failed to read 'reg' property of 'memory' node\n");
		return err;
	}

	return 0;
}
