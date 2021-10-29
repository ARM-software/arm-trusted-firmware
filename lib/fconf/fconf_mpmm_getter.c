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
#include <lib/fconf/fconf_mpmm_getter.h>
#include <libfdt.h>

#include <plat/common/platform.h>

struct fconf_mpmm_config fconf_mpmm_config;
static struct mpmm_topology fconf_mpmm_topology;

/*
 * Within a `cpu` node, determine support for MPMM via the `supports-mpmm`
 * property.
 *
 * Returns `0` on success, or a negative integer representing an error code.
 */
static int fconf_populate_mpmm_cpu(const void *fdt, int off, uintptr_t mpidr)
{
	int ret, len;

	int core_pos;
	struct mpmm_core *core;

	core_pos = plat_core_pos_by_mpidr(mpidr);
	if (core_pos < 0) {
		return -FDT_ERR_BADVALUE;
	}

	core = &fconf_mpmm_topology.cores[core_pos];

	fdt_getprop(fdt, off, "supports-mpmm", &len);
	if (len >= 0) {
		core->supported = true;
		ret = 0;
	} else {
		core->supported = false;
		ret = len;
	}

	return ret;
}

/*
 * Populates the global `fconf_mpmm_config` structure based on what's described
 * by the hardware configuration device tree blob.
 *
 * The device tree is expected to provide a `supports-mpmm` property for each
 * `cpu` node, like so:
 *
 *     cpu@0 {
 *       supports-mpmm;
 *     };
 *
 * This property indicates whether the core implements MPMM, as we cannot detect
 * support for it dynamically.
 */
static int fconf_populate_mpmm(uintptr_t config)
{
	int ret = fdtw_for_each_cpu(
		(const void *)config, fconf_populate_mpmm_cpu);
	if (ret == 0) {
		fconf_mpmm_config.topology = &fconf_mpmm_topology;
	} else {
		ERROR("FCONF: failed to configure MPMM: %d\n", ret);
	}

	return ret;
}

FCONF_REGISTER_POPULATOR(HW_CONFIG, mpmm, fconf_populate_mpmm);
