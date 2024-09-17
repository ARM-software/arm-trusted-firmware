/*
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <common/debug.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <plat_fdt.h>
#include <platform_def.h>

#if defined(XILINX_OF_BOARD_DTB_ADDR)

#define FIT_CONFS_PATH	"/configurations"

static uint8_t is_fit_image(void *dtb)
{
	int64_t confs_noffset;
	uint8_t status = 0;

	confs_noffset = fdt_path_offset(dtb, FIT_CONFS_PATH);
	/*confs_noffset is only present on FIT image */
	if (confs_noffset < 0) {
		status = 0;
	} else {
		status = 1;
	}

	return status;
}

int32_t is_valid_dtb(void *fdt)
{
	int32_t ret = 0;

	if (fdt_check_header(fdt) != 0) {
		ERROR("Can't read DT at %p\n", fdt);
		ret = -FDT_ERR_NOTFOUND;
		goto error;
	}

	ret = fdt_open_into(fdt, fdt, XILINX_OF_BOARD_DTB_MAX_SIZE);
	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", fdt, ret);
		ret = -FDT_ERR_NOTFOUND;
		goto error;
	}

	if (is_fit_image(fdt) != 0U) {
		WARN("FIT image detected, TF-A will not update DTB for DDR address space\n");
		ret = -FDT_ERR_NOTFOUND;
	}
error:
	return ret;
}

static int add_mmap_dynamic_region(unsigned long long base_pa, uintptr_t base_va,
			    size_t size, unsigned int attr)
{
	int ret = 0;
#if defined(PLAT_XLAT_TABLES_DYNAMIC)
	ret = mmap_add_dynamic_region(base_pa, base_va, size, attr);
	if (ret != 0) {
		WARN("Failed to add dynamic region for dtb: error %d\n",
		     ret);
	}
#endif
	return ret;
}

static int remove_mmap_dynamic_region(uintptr_t base_va, size_t size)
{
	int ret = 0;
#if defined(PLAT_XLAT_TABLES_DYNAMIC)
	ret = mmap_remove_dynamic_region(base_va, size);
	if (ret != 0) {
		WARN("Failed to remove dynamic region for dtb:error %d\n",
		     ret);
	}
#endif
	return ret;
}
#endif

#if defined(XILINX_OF_BOARD_DTB_ADDR)
static int check_fdt_reserved_memory(void *dtb, const char *node_name)
{
	int offset = fdt_path_offset(dtb, "/reserved-memory");

	if (offset >= 0) {
		offset = fdt_subnode_offset(dtb, offset, node_name);
	}
	return offset;
}
#endif

void prepare_dtb(void)
{
#if defined(XILINX_OF_BOARD_DTB_ADDR)
	void *dtb;
	int map_ret = 0;
	int ret = 0;

	dtb = (void *)XILINX_OF_BOARD_DTB_ADDR;

	if (!IS_TFA_IN_OCM(BL31_BASE)) {

		map_ret = add_mmap_dynamic_region((unsigned long long)dtb,
						  (uintptr_t)dtb,
						  XILINX_OF_BOARD_DTB_MAX_SIZE,
						  MT_MEMORY | MT_RW | MT_NS);
		if (map_ret == 0) {
			/* Return if no device tree is detected */
			if (is_valid_dtb(dtb) == 0) {
				if (dt_add_psci_node(dtb)) {
					WARN("Failed to add PSCI Device Tree node\n");
				}

				if (dt_add_psci_cpu_enable_methods(dtb)) {
					WARN("Failed to add PSCI cpu enable methods in DT\n");
				}

				/* Check reserved memory set in DT*/
				ret = check_fdt_reserved_memory(dtb, "tf-a");
				if (ret < 0) {
					/* Reserve memory used by Trusted Firmware. */
					ret = fdt_add_reserved_memory(dtb, "tf-a",
							BL31_BASE,
							BL31_LIMIT - BL31_BASE);
					if (ret < 0) {
						WARN("Failed to add reserved memory nodes for BL31 to DT.\n");
					}

				} else {
					WARN("Reserved memory pre-exists in DT.\n");
				}

				ret = fdt_pack(dtb);
				if (ret < 0) {
					WARN("Failed to pack dtb at %p: error %d\n", dtb, ret);
				}
				flush_dcache_range((uintptr_t)dtb, fdt_blob_size(dtb));

				INFO("Changed device tree to advertise PSCI and reserved memories.\n");
			}

			ret = remove_mmap_dynamic_region((uintptr_t)dtb,
							 XILINX_OF_BOARD_DTB_MAX_SIZE);
			if (ret != 0) {
				WARN("Failed to remove mmap dynamic regions.\n");
			}
		}
	}
#endif
}
