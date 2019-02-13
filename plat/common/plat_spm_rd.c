/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/object_pool.h>
#include <plat/common/platform.h>
#include <services/sp_res_desc.h>

/*******************************************************************************
 * Resource pool
 ******************************************************************************/
static struct sp_rd_sect_mem_region rd_mem_regions[PLAT_SPM_MEM_REGIONS_MAX];
static OBJECT_POOL_ARRAY(rd_mem_regions_pool, rd_mem_regions);

static struct sp_rd_sect_notification rd_notifs[PLAT_SPM_NOTIFICATIONS_MAX];
static OBJECT_POOL_ARRAY(rd_notifs_pool, rd_notifs);

static struct sp_rd_sect_service rd_services[PLAT_SPM_SERVICES_MAX];
static OBJECT_POOL_ARRAY(rd_services_pool, rd_services);

/*******************************************************************************
 * Attribute section handler
 ******************************************************************************/
static void rd_parse_attribute(struct sp_rd_sect_attribute *attr,
			       const void *fdt, int node)
{
	int rc = 0;

	/* The minimum size that can be read from the DTB is 32-bit. */
	uint32_t version, sp_type, runtime_el, exec_type;
	uint32_t panic_policy, xlat_granule;

	rc |= fdtw_read_cells(fdt, node, "version", 1, &version);

	if (version != 1) {
		ERROR("Unsupported resource description version: 0x%x\n",
		      version);
		panic();
	}

	rc |= fdtw_read_cells(fdt, node, "sp_type", 1, &sp_type);
	rc |= fdtw_read_cells(fdt, node, "pe_mpidr", 1, &attr->pe_mpidr);
	rc |= fdtw_read_cells(fdt, node, "runtime_el", 1, &runtime_el);
	rc |= fdtw_read_cells(fdt, node, "exec_type", 1, &exec_type);
	rc |= fdtw_read_cells(fdt, node, "panic_policy", 1, &panic_policy);
	rc |= fdtw_read_cells(fdt, node, "xlat_granule", 1, &xlat_granule);
	rc |= fdtw_read_cells(fdt, node, "binary_size", 1, &attr->binary_size);
	rc |= fdtw_read_cells(fdt, node, "load_address", 2, &attr->load_address);
	rc |= fdtw_read_cells(fdt, node, "entrypoint", 2, &attr->entrypoint);

	attr->version = version;
	attr->sp_type = sp_type;
	attr->runtime_el = runtime_el;
	attr->exec_type = exec_type;
	attr->panic_policy = panic_policy;
	attr->xlat_granule = xlat_granule;

	VERBOSE(" Attribute Section:\n");
	VERBOSE("  version: 0x%x\n", version);
	VERBOSE("  sp_type: 0x%x\n", sp_type);
	VERBOSE("  pe_mpidr: 0x%x\n", attr->pe_mpidr);
	VERBOSE("  runtime_el: 0x%x\n", runtime_el);
	VERBOSE("  exec_type: 0x%x\n", exec_type);
	VERBOSE("  panic_policy: 0x%x\n", panic_policy);
	VERBOSE("  xlat_granule: 0x%x\n", xlat_granule);
	VERBOSE("  binary_size: 0x%x\n", attr->binary_size);
	VERBOSE("  load_address: 0x%llx\n", attr->load_address);
	VERBOSE("  entrypoint: 0x%llx\n", attr->entrypoint);

	if (rc) {
		ERROR("Failed to read attribute node elements.\n");
		panic();
	}
}

/*******************************************************************************
 * Memory regions section handlers
 ******************************************************************************/
static void rd_parse_memory_region(struct sp_rd_sect_mem_region *rdmem,
				   const void *fdt, int node)
{
	int rc = 0;
	char name[RD_MEM_REGION_NAME_LEN];

	rc |= fdtw_read_string(fdt, node, "str", (char *)&name, sizeof(name));
	rc |= fdtw_read_cells(fdt, node, "attr", 1, &rdmem->attr);
	rc |= fdtw_read_cells(fdt, node, "base", 2, &rdmem->base);
	rc |= fdtw_read_cells(fdt, node, "size", 2, &rdmem->size);

	size_t len = strlcpy(rdmem->name, name, RD_MEM_REGION_NAME_LEN);

	if (len >= RD_MEM_REGION_NAME_LEN) {
		WARN("Memory region name truncated: '%s'\n", name);
	}

	VERBOSE(" Memory Region:\n");
	VERBOSE("  name: '%s'\n", rdmem->name);
	VERBOSE("  attr: 0x%x\n", rdmem->attr);
	VERBOSE("  base: 0x%llx\n", rdmem->base);
	VERBOSE("  size: 0x%llx\n", rdmem->size);

	if (rc) {
		ERROR("Failed to read mem_region node elements.\n");
		panic();
	}
}

static void rd_parse_memory_regions(struct sp_res_desc *rd, const void *fdt,
				    int node)
{
	int child;
	struct sp_rd_sect_mem_region *rdmem, *old_rdmem;

	fdt_for_each_subnode(child, fdt, node) {
		rdmem = pool_alloc(&rd_mem_regions_pool);

		/* Add element to the start of the list */
		old_rdmem = rd->mem_region;
		rd->mem_region = rdmem;
		rdmem->next = old_rdmem;

		rd_parse_memory_region(rdmem, fdt, child);
	}

	if ((child < 0) && (child != -FDT_ERR_NOTFOUND)) {
		ERROR("%d: fdt_for_each_subnode(): %d\n", __LINE__, node);
		panic();
	}
}

/*******************************************************************************
 * Notifications section handlers
 ******************************************************************************/
static void rd_parse_notification(struct sp_rd_sect_notification *rdnot,
				   const void *fdt, int node)
{
	int rc = 0;

	rc |= fdtw_read_cells(fdt, node, "attr", 1, &rdnot->attr);
	rc |= fdtw_read_cells(fdt, node, "pe", 1, &rdnot->pe);

	VERBOSE(" Notification:\n");
	VERBOSE("  attr: 0x%x\n", rdnot->attr);
	VERBOSE("  pe: 0x%x\n", rdnot->pe);

	if (rc) {
		ERROR("Failed to read notification node elements.\n");
		panic();
	}
}

static void rd_parse_notifications(struct sp_res_desc *rd, const void *fdt, int node)
{
	int child;
	struct sp_rd_sect_notification *rdnot, *old_rdnot;

	fdt_for_each_subnode(child, fdt, node) {
		rdnot = pool_alloc(&rd_notifs_pool);

		/* Add element to the start of the list */
		old_rdnot = rd->notification;
		rd->notification = rdnot;
		rdnot->next = old_rdnot;

		rd_parse_notification(rdnot, fdt, child);
	}

	if ((child < 0) && (child != -FDT_ERR_NOTFOUND)) {
		ERROR("%d: fdt_for_each_subnode(): %d\n", __LINE__, child);
		panic();
	}
}

/*******************************************************************************
 * Services section handlers
 ******************************************************************************/
static void rd_parse_service(struct sp_rd_sect_service *rdsvc, const void *fdt,
			     int node)
{
	int rc = 0;

	/* The minimum size that can be read from the DTB is 32-bit. */
	uint32_t accessibility, request_type, connection_quota;

	rc |= fdtw_read_array(fdt, node, "uuid", 4, &rdsvc->uuid);
	rc |= fdtw_read_cells(fdt, node, "accessibility", 1, &accessibility);
	rc |= fdtw_read_cells(fdt, node, "request_type", 1, &request_type);
	rc |= fdtw_read_cells(fdt, node, "connection_quota", 1, &connection_quota);
	rc |= fdtw_read_cells(fdt, node, "sec_mem_size", 1, &rdsvc->secure_mem_size);
	rc |= fdtw_read_cells(fdt, node, "interrupt_num", 1, &rdsvc->interrupt_num);

	rdsvc->accessibility = accessibility;
	rdsvc->request_type = request_type;
	rdsvc->connection_quota = connection_quota;

	VERBOSE(" Service:\n");
	VERBOSE("  uuid: 0x%08x 0x%08x 0x%08x 0x%08x\n", rdsvc->uuid[0],
		rdsvc->uuid[1], rdsvc->uuid[2], rdsvc->uuid[3]);
	VERBOSE("  accessibility: 0x%x\n", accessibility);
	VERBOSE("  request_type: 0x%x\n", request_type);
	VERBOSE("  connection_quota: 0x%x\n", connection_quota);
	VERBOSE("  secure_memory_size: 0x%x\n", rdsvc->secure_mem_size);
	VERBOSE("  interrupt_num: 0x%x\n", rdsvc->interrupt_num);

	if (rc) {
		ERROR("Failed to read attribute node elements.\n");
		panic();
	}
}

static void rd_parse_services(struct sp_res_desc *rd, const void *fdt, int node)
{
	int child;
	struct sp_rd_sect_service *rdsvc, *old_rdsvc;

	fdt_for_each_subnode(child, fdt, node) {
		rdsvc = pool_alloc(&rd_services_pool);

		/* Add element to the start of the list */
		old_rdsvc = rd->service;
		rd->service = rdsvc;
		rdsvc->next = old_rdsvc;

		rd_parse_service(rdsvc, fdt, child);
	}

	if ((child < 0) && (child != -FDT_ERR_NOTFOUND)) {
		ERROR("%d: fdt_for_each_subnode(): %d\n", __LINE__, node);
		panic();
	}
}

/*******************************************************************************
 * Root node handler
 ******************************************************************************/
static void rd_parse_root(struct sp_res_desc *rd, const void *fdt, int root)
{
	int node;
	char *str;

	str = "attribute";
	node = fdt_subnode_offset_namelen(fdt, root, str, strlen(str));
	if (node < 0) {
		ERROR("Root node doesn't contain subnode '%s'\n", str);
		panic();
	} else {
		rd_parse_attribute(&rd->attribute, fdt, node);
	}

	str = "memory_regions";
	node = fdt_subnode_offset_namelen(fdt, root, str, strlen(str));
	if (node < 0) {
		ERROR("Root node doesn't contain subnode '%s'\n", str);
		panic();
	} else {
		rd_parse_memory_regions(rd, fdt, node);
	}

	str = "notifications";
	node = fdt_subnode_offset_namelen(fdt, root, str, strlen(str));
	if (node < 0) {
		WARN("Root node doesn't contain subnode '%s'\n", str);
	} else {
		rd_parse_notifications(rd, fdt, node);
	}

	str = "services";
	node = fdt_subnode_offset_namelen(fdt, root, str, strlen(str));
	if (node < 0) {
		WARN("Root node doesn't contain subnode '%s'\n", str);
	} else {
		rd_parse_services(rd, fdt, node);
	}
}

/*******************************************************************************
 * Platform handler to load resource descriptor blobs into the active Secure
 * Partition context.
 ******************************************************************************/
int plat_spm_sp_rd_load(struct sp_res_desc *rd, const void *ptr, size_t size)
{
	int rc;
	int root_node;

	assert(rd != NULL);
	assert(ptr != NULL);

	INFO("Reading RD blob at address %p\n", ptr);

	rc = fdt_check_header(ptr);
	if (rc != 0) {
		ERROR("Wrong format for resource descriptor blob (%d).\n", rc);
		return -1;
	}

	root_node = fdt_node_offset_by_compatible(ptr, -1, "arm,sp_rd");
	if (root_node < 0) {
		ERROR("Unrecognized resource descriptor blob (%d)\n", rc);
		return -1;
	}

	rd_parse_root(rd, ptr, root_node);

	return 0;
}
