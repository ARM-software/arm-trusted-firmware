/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include "amu_private.h"
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/extensions/amu.h>
#include <lib/fconf/fconf.h>
#include <libfdt.h>

#include <plat/common/platform.h>

static bool amu_topology_populated_ ; /* Whether the topology is valid */
static struct amu_fconf_topology amu_topology_; /* Populated topology cache */

const struct amu_fconf_topology *amu_topology(void)
{
	if (!amu_topology_populated_) {
		return NULL;
	}

	return &amu_topology_;
}

/*
 * Populate the core-specific AMU structure with information retrieved from a
 * device tree.
 *
 * Returns `0` on success, or a negative integer representing an error code.
 */
static int amu_fconf_populate_cpu_amu(const void *fdt, int parent,
				      struct amu_fconf_core *amu)
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
static int amu_fconf_populate_cpu(const void *fdt, int node, uintptr_t mpidr)
{
	int ret;
	int idx;

	uint32_t amu_phandle;
	struct amu_fconf_core *amu;

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
	amu = &amu_topology_.cores[idx];

	return amu_fconf_populate_cpu_amu(fdt, node, amu);
}

/*
 * For every CPU node (`/cpus/cpu@n`) in an FDT, executes a callback passing a
 * pointer to the FDT and the offset of the CPU node. If the return value of the
 * callback is negative, it is treated as an error and the loop is aborted. In
 * this situation, the value of the callback is returned from the function.
 *
 * Returns `0` on success, or a negative integer representing an error code.
 */
static int amu_fconf_foreach_cpu(const void *fdt,
				 int (*callback)(const void *, int, uintptr_t))
{
	int ret = 0;
	int parent, node = 0;

	parent = fdt_path_offset(fdt, "/cpus");
	if (parent < 0) {
		if (parent == -FDT_ERR_NOTFOUND) {
			parent = 0;
		}

		return parent;
	}

	fdt_for_each_subnode(node, fdt, parent) {
		const char *name;
		int len;

		uintptr_t mpidr = 0U;

		name = fdt_get_name(fdt, node, &len);
		if (strncmp(name, "cpu@", 4) != 0) {
			continue;
		}

		ret = fdt_get_reg_props_by_index(fdt, node, 0, &mpidr, NULL);
		if (ret < 0) {
			break;
		}

		ret = callback(fdt, node, mpidr);
		if (ret < 0) {
			break;
		}
	}

	if ((node < 0) && (node != -FDT_ERR_NOTFOUND)) {
		return node;
	}

	return ret;
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
static int amu_fconf_populate(uintptr_t config)
{
	int ret = amu_fconf_foreach_cpu(
		(const void *)config, amu_fconf_populate_cpu);
	if (ret < 0) {
		ERROR("AMU-FCONF: Failed to configure AMU: %d\n", ret);
	} else {
		amu_topology_populated_ = true;
	}

	return ret;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, amu, amu_fconf_populate);
