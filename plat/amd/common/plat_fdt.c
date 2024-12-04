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
#include <plat_xfer_list.h>

#define FIT_CONFS_PATH	"/configurations"

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
