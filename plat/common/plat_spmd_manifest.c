/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <platform_def.h>
#include <services/spm_core_manifest.h>

#define ATTRIBUTE_ROOT_NODE_STR "attribute"

/*******************************************************************************
 * SPMC attribute node parser
 ******************************************************************************/
static int manifest_parse_attribute(spmc_manifest_attribute_t *attr,
				    const void *fdt,
				    int node)
{
	uint32_t val32;
	int rc;

	assert((attr != NULL) && (fdt != NULL));

	rc = fdt_read_uint32(fdt, node, "maj_ver", &attr->major_version);
	if (rc != 0) {
		ERROR("Missing SPCI %s version in SPM Core manifest.\n",
			"major");
		return rc;
	}

	rc = fdt_read_uint32(fdt, node, "min_ver", &attr->minor_version);
	if (rc != 0) {
		ERROR("Missing SPCI %s version in SPM Core manifest.\n",
			"minor");
		return rc;
	}

	rc = fdt_read_uint32(fdt, node, "spmc_id", &val32);
	if (rc != 0) {
		ERROR("Missing SPMC ID in manifest.\n");
		return rc;
	}

	attr->spmc_id = val32 & 0xffff;

	rc = fdt_read_uint32(fdt, node, "exec_state", &attr->exec_state);
	if (rc != 0) {
		NOTICE("%s not specified in SPM Core manifest.\n",
			"Execution state");
	}

	rc = fdt_read_uint32(fdt, node, "binary_size", &attr->binary_size);
	if (rc != 0) {
		NOTICE("%s not specified in SPM Core manifest.\n",
			"Binary size");
	}

	rc = fdt_read_uint64(fdt, node, "load_address", &attr->load_address);
	if (rc != 0) {
		NOTICE("%s not specified in SPM Core manifest.\n",
			"Load address");
	}

	rc = fdt_read_uint64(fdt, node, "entrypoint", &attr->entrypoint);
	if (rc != 0) {
		NOTICE("%s not specified in SPM Core manifest.\n",
			"Entry point");
	}

	VERBOSE("SPM Core manifest attribute section:\n");
	VERBOSE("  version: %u.%u\n", attr->major_version, attr->minor_version);
	VERBOSE("  spmc_id: 0x%x\n", attr->spmc_id);
	VERBOSE("  binary_size: 0x%x\n", attr->binary_size);
	VERBOSE("  load_address: 0x%llx\n", attr->load_address);
	VERBOSE("  entrypoint: 0x%llx\n", attr->entrypoint);

	return 0;
}

/*******************************************************************************
 * Root node handler
 ******************************************************************************/
static int manifest_parse_root(spmc_manifest_attribute_t *manifest,
			       const void *fdt,
			       int root)
{
	int node;

	assert(manifest != NULL);

	node = fdt_subnode_offset_namelen(fdt, root, ATTRIBUTE_ROOT_NODE_STR,
		sizeof(ATTRIBUTE_ROOT_NODE_STR) - 1);
	if (node < 0) {
		ERROR("Root node doesn't contain subnode '%s'\n",
			ATTRIBUTE_ROOT_NODE_STR);
		return node;
	}

	return manifest_parse_attribute(manifest, fdt, node);
}

/*******************************************************************************
 * Platform handler to parse a SPM Core manifest.
 ******************************************************************************/
int plat_spm_core_manifest_load(spmc_manifest_attribute_t *manifest,
				const void *ptr,
				size_t size)
{
	int rc;

	assert(manifest != NULL);
	assert(ptr != NULL);

	INFO("Reading SPM Core manifest at address %p\n", ptr);

	rc = fdt_check_header(ptr);
	if (rc != 0) {
		ERROR("Wrong format for SPM Core manifest (%d).\n", rc);
		return rc;
	}

	rc = fdt_node_offset_by_compatible(ptr, -1,
				"arm,spci-core-manifest-1.0");
	if (rc < 0) {
		ERROR("Unrecognized SPM Core manifest\n");
		return rc;
	}

	return manifest_parse_root(manifest, ptr, rc);
}
