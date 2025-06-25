/*
 * Copyright (c) 2025, Advanced Micro Devices, Inc. All rights reserved.
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

	ret = fdt_check_header(fdt);
	if (ret != 0) {
		ERROR("Can't read DT at %p\n", fdt);
		goto error;
	}

	ret = fdt_open_into(fdt, fdt, XILINX_OF_BOARD_DTB_MAX_SIZE);
	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", fdt, ret);
		goto error;
	}

	if (is_fit_image(fdt)) {
		WARN("FIT image detected, TF-A will not update DTB for DDR address space\n");
		ret = -FDT_ERR_NOTFOUND;
	}
error:
	return ret;
}

/* TODO: Reserve TFA memory in DT through custom TL entry */
void prepare_dtb(void)
{

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

				if (rsv_count == MAX_RESERVE_ADDR_INDICES) {
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
