/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/fconf/fconf_tbbr_getter.h>
#include <libfdt.h>

struct tbbr_dyn_config_t tbbr_dyn_config;

int fconf_populate_tbbr_dyn_config(uintptr_t config)
{
	int err;
	int node;

	/* As libfdt use void *, we can't avoid this cast */
	const void *dtb = (void *)config;

	/* Assert the node offset point to "arm,tb_fw" compatible property */
	const char *compatible_str = "arm,tb_fw";
	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find %s compatible in dtb\n", compatible_str);
		return node;
	}

	/* Locate the disable_auth cell and read the value */
	err = fdtw_read_cells(dtb, node, "disable_auth", 1, &tbbr_dyn_config.disable_auth);
	if (err < 0) {
		WARN("FCONF: Read cell failed for `disable_auth`\n");
		return err;
	}

	/* Check if the value is boolean */
	if ((tbbr_dyn_config.disable_auth != 0U) && (tbbr_dyn_config.disable_auth != 1U)) {
		WARN("Invalid value for `disable_auth` cell %d\n", tbbr_dyn_config.disable_auth);
		return -1;
	}

#if defined(DYN_DISABLE_AUTH)
	if (tbbr_dyn_config.disable_auth == 1)
		dyn_disable_auth();
#endif

	/* Retrieve the Mbed TLS heap details from the DTB */
	err = fdtw_read_cells(dtb, node,
		"mbedtls_heap_addr", 2, &tbbr_dyn_config.mbedtls_heap_addr);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for mbedtls_heap\n");
		return err;
	}

	err = fdtw_read_cells(dtb, node,
		"mbedtls_heap_size", 1, &tbbr_dyn_config.mbedtls_heap_size);
	if (err < 0) {
		ERROR("FCONF: Read cell failed for mbedtls_heap\n");
		return err;
	}

	VERBOSE("FCONF:tbbr.disable_auth cell found with value = %d\n",
					tbbr_dyn_config.disable_auth);
	VERBOSE("FCONF:tbbr.mbedtls_heap_addr cell found with value = %p\n",
					tbbr_dyn_config.mbedtls_heap_addr);
	VERBOSE("FCONF:tbbr.mbedtls_heap_size cell found with value = %zu\n",
					tbbr_dyn_config.mbedtls_heap_size);

	return 0;
}

FCONF_REGISTER_POPULATOR(tbbr, fconf_populate_tbbr_dyn_config);
