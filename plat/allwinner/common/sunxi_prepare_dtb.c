/*
 * Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>

#include <sunxi_private.h>

void sunxi_prepare_dtb(void *fdt)
{
	int ret;

	if (fdt == NULL || fdt_check_header(fdt) != 0) {
		return;
	}

	ret = fdt_open_into(fdt, fdt, 0x10000);
	if (ret < 0) {
		ERROR("Preparing devicetree at %p: error %d\n", fdt, ret);
		return;
	}

#ifdef SUNXI_BL31_IN_DRAM
	/* Reserve memory used by Trusted Firmware. */
	if (fdt_add_reserved_memory(fdt, "tf-a@40000000", BL31_BASE,
				    BL31_LIMIT - BL31_BASE)) {
		WARN("Failed to add reserved memory nodes to DT.\n");
	}
#endif

	if (sunxi_psci_is_scpi()) {
		ret = fdt_add_cpu_idle_states(fdt, sunxi_idle_states);
		if (ret < 0) {
			WARN("Failed to add idle states to DT: %d\n", ret);
		}
	}

	ret = fdt_pack(fdt);
	if (ret < 0) {
		ERROR("Failed to pack devicetree at %p: error %d\n",
		      fdt, ret);
	}

	clean_dcache_range((uintptr_t)fdt, fdt_blob_size(fdt));
	INFO("Changed devicetree.\n");
}
