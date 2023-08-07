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

#include <plat_fdt.h>
#include <platform_def.h>

#if (defined(XILINX_OF_BOARD_DTB_ADDR) && !IS_TFA_IN_OCM(BL31_BASE))
void prepare_dtb(void)
{
	void *dtb = (void *)XILINX_OF_BOARD_DTB_ADDR;
	int ret;

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
	if (fdt_add_reserved_memory(dtb, "tf-a", BL31_BASE, BL31_LIMIT - BL31_BASE + 1)) {
		WARN("Failed to add reserved memory nodes for BL31 to DT.\n");
		return;
	}

	ret = fdt_pack(dtb);
	if (ret < 0) {
		ERROR("Failed to pack Device Tree at %p: error %d\n", dtb, ret);
		return;
	}

	clean_dcache_range((uintptr_t)dtb, fdt_blob_size(dtb));
	INFO("Changed device tree to advertise PSCI and reserved memories.\n");
}
#else
void prepare_dtb(void)
{
}
#endif
