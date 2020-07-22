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
	uint64_t val64;
	uint32_t val32;

	/* As libfdt use void *, we can't avoid this cast */
	const void *dtb = (void *)config;

	/* Assert the node offset point to "arm,tb_fw" compatible property */
	const char *compatible_str = "arm,tb_fw";
	node = fdt_node_offset_by_compatible(dtb, -1, compatible_str);
	if (node < 0) {
		ERROR("FCONF: Can't find `%s` compatible in dtb\n",
						compatible_str);
		return node;
	}

	/* Locate the disable_auth cell and read the value */
	err = fdt_read_uint32(dtb, node, "disable_auth",
					&tbbr_dyn_config.disable_auth);
	if (err < 0) {
		WARN("FCONF: Read %s failed for `%s`\n",
				"cell", "disable_auth");
		return err;
	}

	/* Check if the value is boolean */
	if ((tbbr_dyn_config.disable_auth != 0U) &&
	    (tbbr_dyn_config.disable_auth != 1U)) {
		WARN("Invalid value for `%s` cell %d\n",
			"disable_auth", tbbr_dyn_config.disable_auth);
		return -1;
	}

#if defined(DYN_DISABLE_AUTH)
	if (tbbr_dyn_config.disable_auth == 1)
		dyn_disable_auth();
#endif

	/* Retrieve the Mbed TLS heap details from the DTB */
	err = fdt_read_uint64(dtb, node, "mbedtls_heap_addr", &val64);
	if (err < 0) {
		ERROR("FCONF: Read %s failed for `%s`\n",
			"cell", "mbedtls_heap_addr");
		return err;
	}
	tbbr_dyn_config.mbedtls_heap_addr = (void *)(uintptr_t)val64;

	err = fdt_read_uint32(dtb, node, "mbedtls_heap_size", &val32);
	if (err < 0) {
		ERROR("FCONF: Read %s failed for `%s`\n",
			"cell", "mbedtls_heap_size");
		return err;
	}
	tbbr_dyn_config.mbedtls_heap_size = val32;

#if MEASURED_BOOT
	/* Retrieve BL2 hash data details from the DTB */
	err = fdtw_read_bytes(dtb, node, "bl2_hash_data", TCG_DIGEST_SIZE,
				&tbbr_dyn_config.bl2_hash_data);
	if (err < 0) {
		ERROR("FCONF: Read %s failed for '%s'\n",
				"bytes", "bl2_hash_data");
		return err;
	}
#endif
	VERBOSE("%s%s%s %d\n", "FCONF: `tbbr.", "disable_auth",
		"` cell found with value =", tbbr_dyn_config.disable_auth);
	VERBOSE("%s%s%s %p\n", "FCONF: `tbbr.", "mbedtls_heap_addr",
		"` cell found with value =", tbbr_dyn_config.mbedtls_heap_addr);
	VERBOSE("%s%s%s %zu\n", "FCONF: `tbbr.", "mbedtls_heap_size",
		"` cell found with value =", tbbr_dyn_config.mbedtls_heap_size);
#if MEASURED_BOOT
	VERBOSE("%s%s%s %p\n", "FCONF: `tbbr.", "bl2_hash_data",
		"` array found at address =", tbbr_dyn_config.bl2_hash_data);
#endif
	return 0;
}

FCONF_REGISTER_POPULATOR(TB_FW, tbbr, fconf_populate_tbbr_dyn_config);
