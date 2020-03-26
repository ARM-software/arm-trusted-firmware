/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>
#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <errno.h>
#include <platform_def.h>
#include <services/spm_core_manifest.h>

/*******************************************************************************
 * Attribute section handler
 ******************************************************************************/
static int manifest_parse_attribute(spmc_manifest_sect_attribute_t *attr,
				    const void *fdt,
				    int node)
{
	uint32_t val32;
	int rc = 0;

	assert(attr && fdt);

	rc = fdt_read_uint32(fdt, node, "maj_ver", &attr->major_version);
	if (rc) {
		ERROR("Missing SPCI major version in SPM core manifest.\n");
		return -ENOENT;
	}

	rc = fdt_read_uint32(fdt, node, "min_ver", &attr->minor_version);
	if (rc) {
		ERROR("Missing SPCI minor version in SPM core manifest.\n");
		return -ENOENT;
	}

	rc = fdt_read_uint32(fdt, node, "spmc_id", &val32);
	if (rc) {
		ERROR("Missing SPMC ID in manifest.\n");
		return -ENOENT;
	}
	attr->spmc_id = val32;

	rc = fdt_read_uint32(fdt, node, "exec_state", &attr->exec_state);
	if (rc)
		NOTICE("Execution state not specified in SPM core manifest.\n");

	rc = fdt_read_uint32(fdt, node, "binary_size", &attr->binary_size);
	if (rc)
		NOTICE("Binary size not specified in SPM core manifest.\n");

	rc = fdt_read_uint64(fdt, node, "load_address", &attr->load_address);
	if (rc)
		NOTICE("Load address not specified in SPM core manifest.\n");

	rc = fdt_read_uint64(fdt, node, "entrypoint", &attr->entrypoint);
	if (rc)
		NOTICE("Entrypoint not specified in SPM core manifest.\n");

	VERBOSE("SPM core manifest attribute section:\n");
	VERBOSE("  version: %x.%x\n", attr->major_version, attr->minor_version);
	VERBOSE("  spmc_id: %x\n", attr->spmc_id);
	VERBOSE("  binary_size: 0x%x\n", attr->binary_size);
	VERBOSE("  load_address: 0x%llx\n", attr->load_address);
	VERBOSE("  entrypoint: 0x%llx\n", attr->entrypoint);

	return 0;
}

/*******************************************************************************
 * Root node handler
 ******************************************************************************/
static int manifest_parse_root(spmc_manifest_sect_attribute_t *manifest,
				const void *fdt,
				int root)
{
	int node;
	char *str;

	str = "attribute";
	node = fdt_subnode_offset_namelen(fdt, root, str, strlen(str));
	if (node < 0) {
		ERROR("Root node doesn't contain subnode '%s'\n", str);
		return -ENOENT;
	}

	return manifest_parse_attribute(manifest, fdt, node);
}

/*******************************************************************************
 * Platform handler to parse a SPM core manifest.
 ******************************************************************************/
int plat_spm_core_manifest_load(spmc_manifest_sect_attribute_t *manifest,
				const void *ptr,
				size_t size)
{
	int rc;
	int root_node;

	assert(manifest != NULL);
	assert(ptr != NULL);

	INFO("Reading SPM core manifest at address %p\n", ptr);

	rc = fdt_check_header(ptr);
	if (rc != 0) {
		ERROR("Wrong format for SPM core manifest (%d).\n", rc);
		return -EINVAL;
	}

	INFO("Reading SPM core manifest at address %p\n", ptr);

	root_node = fdt_node_offset_by_compatible(ptr, -1,
				"arm,spci-core-manifest-1.0");
	if (root_node < 0) {
		ERROR("Unrecognized SPM core manifest\n");
		return -ENOENT;
	}

	INFO("Reading SPM core manifest at address %p\n", ptr);
	return manifest_parse_root(manifest, ptr, root_node);
}
