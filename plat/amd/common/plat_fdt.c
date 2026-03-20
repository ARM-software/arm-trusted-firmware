/*
 * Copyright (c) 2025-2026, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <common/debug.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <platform_def.h>

#include <plat_fdt.h>
#ifdef TRANSFER_LIST
#include <plat_xfer_list.h>
#endif

#define FIT_CONFS_PATH	"/configurations"

#define DTBO_BUFF_MAX	SZ_4K
static uint8_t ovrly_buff[DTBO_BUFF_MAX+TL_FDT_OVRLY_SUBTYPE_SIZE] __aligned(8);

static struct reserve_mem_range rsvnodes[MAX_RESERVE_ADDR_INDICES] = {};
static uint32_t rsv_count;

static bool is_fit_image(void *dtb)
{
	int64_t confs_noffset = 0;
	bool status = true;

	confs_noffset = fdt_path_offset(dtb, FIT_CONFS_PATH);

	/* confs_noffset is only present on FIT image */
	if (confs_noffset < 0) {
		status = false;
	}

	return status;
}

int32_t is_valid_dtb(void *fdt)
{
	int32_t ret = 0;

	if (fdt == NULL) {
		ERROR("Invalid DT address\n");
		ret = -FDT_ERR_NOTFOUND;
		goto error;
	}

	ret = fdt_check_header(fdt);
	if (ret != 0) {
		ERROR("Can't read DT at %p\n", fdt);
		goto error;
	}

	if (is_fit_image(fdt)) {
		WARN("FIT image detected, TF-A will not update DTB for DDR address space\n");
		ret = -FDT_ERR_NOTFOUND;
	}
error:
	return ret;
}

static int32_t is_node_reserved(const char *node_name, uint64_t base, uint64_t limit)
{
	void *dtb = (void *)plat_retrieve_dt_addr();
	uint64_t rbase = 0, rsize = 0;
	int32_t ret = -FDT_ERR_NOTFOUND;
	int32_t subnode;
	uint64_t rend;
	int32_t node;
	int32_t rm;

	assert(node_name != NULL);

	if (is_valid_dtb(dtb) < 0) {
		ret = -FDT_ERR_BADSTRUCTURE;
		goto exit;
	}

	if (limit > base) {
		rm = fdt_path_offset(dtb, "/reserved-memory");
		if (rm < 0) {
			ret = rm;
			goto exit;
		}

		/* Check if node is already present */
		subnode = fdt_subnode_offset(dtb, rm, node_name);
		if (subnode >= 0) {
			ret = 0;
			goto exit;
		}

		/* Covered by any existing reserved no-map node? */
		fdt_for_each_subnode(node, dtb, rm) {
			if (!fdt_getprop(dtb, node, "no-map", NULL)) {
				continue;
			}

			if (fdt_get_reg_props_by_index(dtb, node, 0, &rbase, &rsize) < 0) {
				continue;
			}

			rend = rbase + rsize;
			if (base >= rbase && limit <= rend) {
				/* Node region is already reserved */
				ret = 0;
				break;
			}
		}
	}
exit:
	return ret;
}

void prepare_dtb(void)
{
	uint64_t size = (uint64_t)(BL31_LIMIT - BL31_BASE);
	uint64_t base = (uint64_t)BL31_BASE;
	uint32_t two = cpu_to_fdt32(2);
	const void *compat = NULL;
	int32_t compat_len = 0;
	void *base_dtb = NULL;
	uint32_t reg_cells[4];
	size_t ovrly_buff_len;
	void *fdt = NULL;
	int32_t rc = 0;

	rc = is_node_reserved("tf-a", (uint64_t)BL31_BASE, (uint64_t)BL31_LIMIT);
	if (rc == 0) {
		INFO("TF-A is already reserved\n");
		goto exit;
	}

	base_dtb = (void *)plat_retrieve_dt_addr();
	if (base_dtb == NULL) {
		WARN("Failed to retrieve base DTB for overlay compatible\n");
		rc = -1;
		goto exit;
	}

	compat = fdt_getprop(base_dtb, 0, "compatible", &compat_len);
	if (compat == NULL || compat_len <= 0) {
		WARN("No compatible string in base DTB\n");
		rc = -1;
		goto exit;
	}

	memset(ovrly_buff, 0, sizeof(ovrly_buff));
	*(uint64_t *)ovrly_buff = TL_FDT_OVRLY_SUBTYPE;
	fdt = (void *)(ovrly_buff + TL_FDT_OVRLY_SUBTYPE_SIZE);
	rc = fdt_create(fdt, DTBO_BUFF_MAX);
	if (rc < 0) {
		goto exit;
	}

	rc = fdt_finish_reservemap(fdt);
	if (rc < 0) {
		goto exit;
	}

	rc = fdt_begin_node(fdt, "");
	if (rc < 0) {
		goto exit;
	}

	rc = fdt_property(fdt, "compatible", compat, compat_len);
	if (rc < 0) {
		WARN("Failed to set overlay compatible string\n");
		goto exit;
	}

	/* fragment@0 */
	rc = fdt_begin_node(fdt, "fragment@0");
	if (rc < 0) {
		WARN("Failed to create DT fragment\n");
		goto exit;
	}

	rc = fdt_property_string(fdt, "target-path", "/");
	if (rc < 0) {
		WARN("Failed to get target path\n");
		goto exit;
	}

	rc = fdt_begin_node(fdt, "__overlay__");
	if (rc < 0) {
		WARN("Failed to add overlay node\n");
		goto exit;
	}

	/* reserved-memory node */
	rc = fdt_begin_node(fdt, "reserved-memory");
	if (rc < 0) {
		WARN("Failed to add reserve-memory node\n");
		goto exit;
	}

	rc = fdt_property(fdt, "#address-cells", &two, sizeof(two));
	if (rc < 0) {
		goto exit;
	}
	rc = fdt_property(fdt, "#size-cells", &two, sizeof(two));
	if (rc < 0) {
		goto exit;
	}
	rc = fdt_property(fdt, "ranges", NULL, 0);
	if (rc < 0) {
		goto exit;
	}

	rc = fdt_begin_node(fdt, "tf-a");
	if (rc < 0) {
		WARN("Failed to add tf-a reserve node\n");
		goto exit;
	}

	/* reg = <base size> as 2 x 64-bit => 4 cells */
	reg_cells[0] = cpu_to_fdt32((uint32_t)(base >> 32));
	reg_cells[1] = cpu_to_fdt32((uint32_t)(base & 0xFFFFFFFF));
	reg_cells[2] = cpu_to_fdt32((uint32_t)(size >> 32));
	reg_cells[3] = cpu_to_fdt32((uint32_t)(size & 0xFFFFFFFF));

	rc = fdt_property(fdt, "reg", reg_cells, sizeof(reg_cells));
	if (rc < 0) {
		WARN("Failed to set prop reg in the reserve node\n");
		goto exit;
	}

	rc = fdt_property(fdt, "no-map", NULL, 0);
	if (rc < 0) {
		WARN("Failed to set prop no-map in the reserve node\n");
		goto exit;
	}

	/* tf-a node end */
	rc = fdt_end_node(fdt);
	if (rc < 0) {
		WARN("Failed to add end node\n");
		goto exit;
	}

	/* reserve-memory node end */
	rc = fdt_end_node(fdt);
	if (rc < 0) {
		WARN("Failed to add reserved end node\n");
		goto exit;
	}

	/* overlay node end */
	rc = fdt_end_node(fdt);
	if (rc < 0) {
		WARN("Failed to add overlay end node\n");
		goto exit;
	}

	/* fragment0 node end */
	rc = fdt_end_node(fdt);
	if (rc < 0) {
		WARN("Failed to add fragment@0 end node\n");
		goto exit;
	}

	/* root node end */
	rc = fdt_end_node(fdt);
	if (rc < 0) {
		goto exit;
	}

	rc = fdt_finish(fdt);
	if (rc < 0) {
		goto exit;
	}

	rc = fdt_pack(fdt);
	if (rc < 0) {
		goto exit;
	}

	ovrly_buff_len = TL_FDT_OVRLY_SUBTYPE_SIZE + fdt_totalsize(fdt);

	/* overlay blob to normal world via transfer list */
	rc = tl_add_dt_overlay(ovrly_buff, ovrly_buff_len);
	if (rc != 0) {
		WARN("Failed to add overlay to TL\n");
	}
exit:
	VERBOSE("Overlay creation returned: %d\n", rc);
}

uintptr_t plat_retrieve_dt_addr(void)
{
	void *dtb = NULL;

	dtb = transfer_list_retrieve_dt_address();
	if (dtb == NULL) {
		WARN("TL header or DT entry is invalid\n");
	}

	return (uintptr_t)dtb;
}

struct reserve_mem_range *get_reserved_entries_fdt(uint32_t *reserve_nodes)
{
	struct reserve_mem_range *rsvmr = NULL;

	if ((rsv_count > 0) && (reserve_nodes != NULL)) {
		rsvmr = &rsvnodes[0];
		*reserve_nodes = rsv_count;
	}

	return rsvmr;
}

/* TODO: Parse TL overlays for updated tf-a and op-tee reserved nodes */
uint32_t retrieve_reserved_entries(void)
{
	uint32_t ret = 1;
	void *dtb = NULL;
	int offset, node;
	uint32_t i = 0;
	const fdt32_t *reg_prop;


	/* Get DT blob address */
	dtb = (void *)plat_retrieve_dt_addr();

	/* Check if DT is valid */
	if (is_valid_dtb(dtb) >= 0) {
		/* Find reserved memory node */
		offset = fdt_path_offset(dtb, "/reserved-memory");
		if (offset >= 0) {

			/* Parse subnodes of reserved-memory */
			fdt_for_each_subnode(node, dtb, offset) {
				if (fdt_getprop(dtb, node, "no-map", NULL) == NULL) {
					continue;
				}

				if (i == MAX_RESERVE_ADDR_INDICES) {
					break;
				}

				reg_prop = fdt_getprop(dtb, node, "reg", NULL);
				if (reg_prop == NULL) {
					INFO("No valid reg prop found for subnode\n");
					continue;
				}

				rsvnodes[i].base = (((uint64_t)fdt32_to_cpu(reg_prop[0]) << 32) |
						fdt32_to_cpu(reg_prop[1]));
				rsvnodes[i].size = (((uint64_t)fdt32_to_cpu(reg_prop[2]) << 32) |
						fdt32_to_cpu(reg_prop[3]));
				i++;
			}
			ret = 0;
			rsv_count = i;
		}
	}

	return ret;
}
