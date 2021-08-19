/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_amu_getter.h>
#include <libfdt.h>

#include <plat/common/platform.h>

struct fconf_amu_config fconf_amu_config;
static struct amu_topology fconf_amu_topology_;

/*
 * Populate the core-specific AMU structure with information retrieved from a
 * device tree.
 *
 * Returns `0` on success, or a negative integer representing an error code.
 */
static int fconf_populate_amu_cpu_amu(const void *fdt, int parent,
				      struct amu_core *amu)
{
	int ret = 0;
	int node = 0;

	fdt_for_each_subnode(node, fdt, parent) {
		const char *name;
		const char *value;
		int len;

		uintptr_t idx = 0U;

		name = fdt_get_name(fdt, node, &len);
		if (strncmp(name, "counter@", 8) != 0) {
			continue;
		}

		ret = fdt_get_reg_props_by_index(fdt, node, 0, &idx, NULL);
		if (ret < 0) {
			break;
		}

		value = fdt_getprop(fdt, node, "enable-at-el3", &len);
		if ((value == NULL) && (len != -FDT_ERR_NOTFOUND)) {
			break;
		}

		if (len != -FDT_ERR_NOTFOUND) {
			amu->enable |= (1 << idx);
		}
	}

	if ((node < 0) && (node != -FDT_ERR_NOTFOUND)) {
		return node;
	}

	return ret;
}

/*
 * Within a `cpu` node, attempt to dereference the `amu` property, and populate
 * the AMU information for the core.
 *
 * Returns `0` on success, or a negative integer representing an error code.
 */
static int fconf_populate_amu_cpu(const void *fdt, int node, uintptr_t mpidr)
{
	int ret;
	int idx;

	uint32_t amu_phandle;
	struct amu_core *amu;

	ret = fdt_read_uint32(fdt, node, "amu", &amu_phandle);
	if (ret < 0) {
		if (ret == -FDT_ERR_NOTFOUND) {
			ret = 0;
		}

		return ret;
	}

	node = fdt_node_offset_by_phandle(fdt, amu_phandle);
	if (node < 0) {
		return node;
	}

	idx = plat_core_pos_by_mpidr(mpidr);
	if (idx < 0) {
		return -FDT_ERR_BADVALUE;
	}

	amu = &fconf_amu_topology_.cores[idx];

	return fconf_populate_amu_cpu_amu(fdt, node, amu);
}

/*
 * Populates the global `amu_topology` structure based on what's described by
 * the hardware configuration device tree blob.
 *
 * The device tree is expected to provide an `amu` property for each `cpu` node,
 * like so:
 *
 *     cpu@0 {
 *         amu = <&cpu0_amu>;
 *     };
 *
 *     amus {
 *         cpu0_amu: amu-0 {
 *             counters {
 *                 #address-cells = <2>;
 *                 #size-cells = <0>;
 *
 *                 counter@x,y {
 *                     reg = <x y>; // Group x, counter y
 *                 };
 *             };
 *         };
 *     };
 */
static int fconf_populate_amu(uintptr_t config)
{
	int ret = fdtw_for_each_cpu(
		(const void *)config, fconf_populate_amu_cpu);
	if (ret == 0) {
		fconf_amu_config.topology = &fconf_amu_topology_;
	} else {
		ERROR("FCONF: failed to parse AMU information: %d\n", ret);
	}

	return ret;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, amu, fconf_populate_amu);
