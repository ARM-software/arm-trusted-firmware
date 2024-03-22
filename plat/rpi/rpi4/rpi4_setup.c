/*
 * Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>

#include <rpi_shared.h>

/*
 * Remove the FDT /memreserve/ entry that covers the region at the very
 * beginning of memory (if that exists). This is where the secondaries
 * originally spin, but we pull them out there.
 * Having overlapping /reserved-memory and /memreserve/ regions confuses
 * the Linux kernel, so we need to get rid of this one.
 */
static void remove_spintable_memreserve(void *dtb)
{
	uint64_t addr, size;
	int regions = fdt_num_mem_rsv(dtb);
	int i;

	for (i = 0; i < regions; i++) {
		if (fdt_get_mem_rsv(dtb, i, &addr, &size) != 0) {
			return;
		}
		if (size == 0U) {
			return;
		}
		/* We only look for the region at the beginning of DRAM. */
		if (addr != 0U) {
			continue;
		}
		/*
		 * Currently the region in the existing DTs is exactly 4K
		 * in size. Should this value ever change, there is probably
		 * a reason for that, so inform the user about this.
		 */
		if (size == 4096U) {
			fdt_del_mem_rsv(dtb, i);
			return;
		}
		WARN("Keeping unknown /memreserve/ region at 0, size: %" PRId64 "\n",
		     size);
	}
}

static void rpi4_prepare_dtb(void)
{
	void *dtb = (void *)rpi4_get_dtb_address();
	uint32_t gic_int_prop[3];
	int ret, offs;

	/* Return if no device tree is detected */
	if (fdt_check_header(dtb) != 0)
		return;

	ret = fdt_open_into(dtb, dtb, 0x100000);
	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", dtb, ret);
		return;
	}

	if (dt_add_psci_node(dtb)) {
		ERROR("Failed to add PSCI Device Tree node\n");
		return;
	}

	if (dt_add_psci_cpu_enable_methods(dtb)) {
		ERROR("Failed to add PSCI cpu enable methods in Device Tree\n");
		return;
	}

	/*
	 * Remove the original reserved region (used for the spintable), and
	 * replace it with a region describing the whole of Trusted Firmware.
	 */
	remove_spintable_memreserve(dtb);
	if (fdt_add_reserved_memory(dtb, "atf@0", 0, 0x80000))
		WARN("Failed to add reserved memory nodes to DT.\n");

	offs = fdt_node_offset_by_compatible(dtb, 0, "arm,gic-400");
	gic_int_prop[0] = cpu_to_fdt32(1);		// PPI
	gic_int_prop[1] = cpu_to_fdt32(9);		// PPI #9
	gic_int_prop[2] = cpu_to_fdt32(0x0f04);		// all cores, level high
	fdt_setprop(dtb, offs, "interrupts", gic_int_prop, 12);

	offs = fdt_path_offset(dtb, "/chosen");
	fdt_setprop_string(dtb, offs, "stdout-path", "serial0");

	ret = fdt_pack(dtb);
	if (ret < 0)
		ERROR("Failed to pack Device Tree at %p: error %d\n", dtb, ret);

	clean_dcache_range((uintptr_t)dtb, fdt_blob_size(dtb));
	INFO("Changed device tree to advertise PSCI.\n");
}

void plat_rpi_bl31_custom_setup(void)
{
	rpi4_prepare_dtb();
}
