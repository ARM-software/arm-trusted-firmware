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

void prepare_dtb(void)
{
	void *dtb;
	int ret;
#if !defined(XILINX_OF_BOARD_DTB_ADDR)
	return;
#else
	dtb = (void *)XILINX_OF_BOARD_DTB_ADDR;
#endif
	if (IS_TFA_IN_OCM(BL31_BASE))
		return;

#if defined(PLAT_XLAT_TABLES_DYNAMIC)
	ret = mmap_add_dynamic_region((unsigned long long)dtb,
				      (uintptr_t)dtb,
				      XILINX_OF_BOARD_DTB_MAX_SIZE,
				      MT_MEMORY | MT_RW | MT_NS);
	if (ret != 0) {
		WARN("Failed to add dynamic region for dtb: error %d\n", ret);
		return;
	}
#endif

	/* Return if no device tree is detected */
	if (fdt_check_header(dtb) != 0) {
		NOTICE("Can't read DT at %p\n", dtb);
		return;
	}

	ret = fdt_open_into(dtb, dtb, XILINX_OF_BOARD_DTB_MAX_SIZE);
	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", dtb, ret);
		return;
	}

	/* Reserve memory used by Trusted Firmware. */
	if (fdt_add_reserved_memory(dtb, "tf-a", BL31_BASE, BL31_LIMIT - BL31_BASE)) {
		WARN("Failed to add reserved memory nodes for BL31 to DT.\n");
		return;
	}

	ret = fdt_pack(dtb);
	if (ret < 0) {
		ERROR("Failed to pack Device Tree at %p: error %d\n", dtb, ret);
		return;
	}

	clean_dcache_range((uintptr_t)dtb, fdt_blob_size(dtb));

#if defined(PLAT_XLAT_TABLES_DYNAMIC)
	ret = mmap_remove_dynamic_region((uintptr_t)dtb,
					 XILINX_OF_BOARD_DTB_MAX_SIZE);
	if (ret != 0) {
		WARN("Failed to remove dynamic region for dtb: error %d\n", ret);
		return;
	}
#endif

	INFO("Changed device tree to advertise PSCI and reserved memories.\n");
}
