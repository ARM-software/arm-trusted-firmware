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
#if defined(XILINX_OF_BOARD_DTB_ADDR)
	void *dtb;
	int map_ret = 0;
	int ret = 0;

	dtb = (void *)XILINX_OF_BOARD_DTB_ADDR;

	if (!IS_TFA_IN_OCM(BL31_BASE)) {

#if defined(PLAT_XLAT_TABLES_DYNAMIC)
		map_ret = mmap_add_dynamic_region((unsigned long long)dtb,
						 (uintptr_t)dtb,
						 XILINX_OF_BOARD_DTB_MAX_SIZE,
						 MT_MEMORY | MT_RW | MT_NS);
		if (map_ret != 0) {
			WARN("Failed to add dynamic region for dtb: error %d\n",
			     map_ret);
		}
#endif

		if (!map_ret) {
			/* Return if no device tree is detected */
			if (fdt_check_header(dtb) != 0) {
				NOTICE("Can't read DT at %p\n", dtb);
			} else {
				ret = fdt_open_into(dtb, dtb, XILINX_OF_BOARD_DTB_MAX_SIZE);

				if (ret < 0) {
					ERROR("Invalid Device Tree at %p: error %d\n",
					      dtb, ret);
				} else {

					if (dt_add_psci_node(dtb)) {
						WARN("Failed to add PSCI Device Tree node\n");
					}

					if (dt_add_psci_cpu_enable_methods(dtb)) {
						WARN("Failed to add PSCI cpu enable methods in DT\n");
					}

					/* Reserve memory used by Trusted Firmware. */
					ret = fdt_add_reserved_memory(dtb,
								     "tf-a",
								     BL31_BASE,
								     BL31_LIMIT
								     -
								     BL31_BASE);
					if (ret < 0) {
						WARN("Failed to add reserved memory nodes for BL31 to DT.\n");
					}

					ret = fdt_pack(dtb);
					if (ret < 0) {
						WARN("Failed to pack dtb at %p: error %d\n",
						     dtb, ret);
					}
					flush_dcache_range((uintptr_t)dtb,
							   fdt_blob_size(dtb));

					INFO("Changed device tree to advertise PSCI and reserved memories.\n");

				}
			}

		}


#if defined(PLAT_XLAT_TABLES_DYNAMIC)
		if (!map_ret) {
			ret = mmap_remove_dynamic_region((uintptr_t)dtb,
					 XILINX_OF_BOARD_DTB_MAX_SIZE);
			if (ret != 0) {
				WARN("Failed to remove dynamic region for dtb:error %d\n",
					ret);
			}
		}
#endif
	}

#endif
}
