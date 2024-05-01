/*
 * Copyright (c) 2024, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Amend the device tree to adjust the L2 cache size, which is different
 * between the revisions of the H616 chips: earlier versions have 256 KB of L2,
 * later versions 1 MB.
 * Read the cache ID registers and adjust the size and number of sets entries
 * in the L2 cache DT node.
 */

#include <common/fdt_wrappers.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#define CACHE_L1D		0x0
#define CACHE_L1I		0x1
#define CACHE_L2U		0x2

#define CCSIDR_SETS_SHIFT	13
#define CCSIDR_SETS_MASK	GENMASK(14, 0)
#define CCSIDR_ASSOC_SHIFT	3
#define CCSIDR_ASSOC_MASK	GENMASK(9, 0)
#define CCSIDR_LSIZE_SHIFT	0
#define CCSIDR_LSIZE_MASK	GENMASK(2, 0)

static uint32_t armv8_get_ccsidr(unsigned int sel)
{
	uint32_t reg;

	__asm__ volatile ("msr CSSELR_EL1, %0\n" :: "r" (sel));
	__asm__ volatile ("mrs %0, CCSIDR_EL1\n" : "=r" (reg));

	return reg;
}

void sunxi_soc_fdt_fixup(void *dtb)
{
	int node = fdt_path_offset(dtb, "/cpus/cpu@0");
	uint32_t phandle, ccsidr, cell;
	int sets, line_size, assoc;
	int ret;

	if (node < 0) {
		return;
	}

	ret = fdt_read_uint32(dtb, node, "next-level-cache", &phandle);
	if (ret != 0) {
		return;
	}

	node = fdt_node_offset_by_phandle(dtb, phandle);
	if (ret != 0) {
		return;
	}

	ccsidr = armv8_get_ccsidr(CACHE_L2U);
	sets = ((ccsidr >> CCSIDR_SETS_SHIFT) & CCSIDR_SETS_MASK) + 1;
	line_size = 16U << ((ccsidr >> CCSIDR_LSIZE_SHIFT) & CCSIDR_LSIZE_MASK);
	assoc = ((ccsidr >> CCSIDR_ASSOC_SHIFT) & CCSIDR_ASSOC_MASK) + 1;

	cell = cpu_to_fdt32(sets);
	fdt_setprop(dtb, node, "cache-sets", &cell, sizeof(cell));

	cell = cpu_to_fdt32(line_size);
	fdt_setprop(dtb, node, "cache-line-size", &cell, sizeof(cell));

	cell = cpu_to_fdt32(sets * assoc * line_size);
	fdt_setprop(dtb, node, "cache-size", &cell, sizeof(cell));
}
