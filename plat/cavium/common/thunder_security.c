/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <platform_def.h>
#include <thunder_private.h>
#include <stdio.h>
#include <debug.h>

#define MAX_WAYS 16

/* Flush the L2 Cache */
static void l2c_flush(void)
{
	/* Select the L2 cache */
	union cavm_ap_csselr_el1 csselr_el1;
	union cavm_ap_ccsidr_el1 ccsidr_el1;
	unsigned int sets, ways;
	int l2_way, l2_set;
	uint64_t val;

	csselr_el1.s.level = 1;

	__asm__ volatile ("msr csselr_el1, %0" : : "r"(csselr_el1.u));

	__asm__ volatile ("mrs %0, ccsidr_el1" : "=&r"(ccsidr_el1.u));

	sets = ccsidr_el1.s.numsets + 1;
	ways = ccsidr_el1.s.associativity + 1;

	int is_rtg = 1; /* Clear remote tags */

	for (l2_way = 0; l2_way < ways; l2_way++) {
		for (l2_set = 0; l2_set < sets; l2_set++) {
			val = 128 * (l2_set + sets * (l2_way + (is_rtg * MAX_WAYS)));
			__asm__ volatile("sys #0,c11,C0,#5, %0\n" : : "r"(val));
		}
	}

	is_rtg = 0; /* Clear local tags */
	for (l2_way = 0; l2_way < ways; l2_way++) {
		for (l2_set = 0; l2_set < sets; l2_set++) {
			val = 128 * (l2_set + sets * (l2_way + (is_rtg * MAX_WAYS)));
			__asm__ volatile("sys #0,c11,C0,#5, %0\n" : : "r"(val));
		}
	}
}

struct l2c_region {
	unsigned int  node;
	unsigned int  number;
	unsigned long start;
	unsigned long end;
	unsigned int  secure;
};

struct l2c_region l2c_map [] = {
	{
		.node    = 0,
		.number  = 0,
		.start   = TZDRAM_BASE,
		.end     = TZDRAM_BASE + TZDRAM_SIZE,
		.secure  = 1,
	},
	{
		.node    = 0,
		.number  = 1,
		.start   = TZDRAM_BASE + TZDRAM_SIZE,
		.end     = ~0UL,
		.secure  = 0,
	},
	{
		.node    = 1,
		.number  = 0,
		.start   = 0,
		.end     = ~0UL,
		.secure  = 0,
	},
	{
		.node    = ~0U,
	},
};

void thunder_security_setup(void)
{
	unsigned node_count = thunder_get_node_count();
	union cavm_l2c_asc_regionx_attr l2c_asc_attr;
	struct l2c_region *region = l2c_map;

	uint64_t dram_end;

	while (region->node < node_count) {
		dram_end = thunder_dram_size_node(region->node) - 1;

		if (region->end > dram_end)
			region->end = dram_end;

		CSR_WRITE_PA(region->node, CAVM_L2C_ASC_REGIONX_START(region->number), region->start);
		CSR_WRITE_PA(region->node, CAVM_L2C_ASC_REGIONX_END(region->number), region->end);

		l2c_asc_attr.u = CSR_READ_PA(region->node, CAVM_L2C_ASC_REGIONX_ATTR(region->number));
		l2c_asc_attr.s.s_en  = region->secure;
		l2c_asc_attr.s.ns_en = !region->secure;

		CSR_WRITE_PA(region->node, CAVM_L2C_ASC_REGIONX_ATTR(region->number), l2c_asc_attr.u);

		INFO("Mark memory region %d at node %d:: %llx to %llx as %ssecure (%llx)\n",
			region->number, region->node,
			CSR_READ_PA(region->node, CAVM_L2C_ASC_REGIONX_START(region->number)),
			CSR_READ_PA(region->node, CAVM_L2C_ASC_REGIONX_END(region->number)),
			region->secure ? "" : "non-",
			CSR_READ_PA(region->node, CAVM_L2C_ASC_REGIONX_ATTR(region->number)));

		region++;
	}

	VERBOSE("Flushing L1C\n");
	dcsw_op_all(DCCISW);

	VERBOSE("Flushing L2C\n");
	l2c_flush();

	VERBOSE("Flushing IC\n");
	__asm__ volatile("ic iallu\n"
		     "isb\n");
}
