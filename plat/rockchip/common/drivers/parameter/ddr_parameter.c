/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <plat_private.h>
#include <soc.h>

#include "ddr_parameter.h"

/*
 *  The miniloader delivers the parameters about ddr usage info from address
 * 0x02000000 and the data format is defined as below figure. It tells ATF the
 * areas of ddr that are used by platform, we treat them as non-secure regions
 * by default. Then we should parse the other part regions and configurate them
 * as secure regions to avoid illegal access.
 *
 *			[ddr usage info data format]
 * 0x02000000
 * -----------------------------------------------------------------------------
 * |       <name>        |  <size>   |      <description>                      |
 * -----------------------------------------------------------------------------
 * | count               |  4byte    | the array numbers of the                |
 * |                     |           | 'addr_array' and 'size_array'           |
 * -----------------------------------------------------------------------------
 * | reserved            |  4byte    | just for 'addr_array' 8byte aligned     |
 * -----------------------------------------------------------------------------
 * | addr_array[count]   | per 8byte | memory region base address              |
 * -----------------------------------------------------------------------------
 * | size_array[count]   | per 8byte | memory region size (byte)               |
 * -----------------------------------------------------------------------------
 */

/*
 * function: read parameters info(ns-regions) and try to parse s-regions info
 *
 * @addr: head address to the ddr usage struct from miniloader
 * @max_mb: the max ddr capacity(MB) that the platform support
 */
struct param_ddr_usage ddr_region_usage_parse(uint64_t addr, uint64_t max_mb)
{
	uint64_t base, top;
	uint32_t i, addr_offset, size_offset;
	struct param_ddr_usage p;

	memset(&p, 0, sizeof(p));

	/* read how many blocks of ns-regions, read from offset: 0x0 */
	p.ns_nr = mmio_read_32(addr + REGION_NR_OFFSET);
	if ((p.ns_nr > DDR_REGION_NR_MAX) || (p.ns_nr == 0)) {
		ERROR("over or zero region, nr=%d, max=%d\n",
		      p.ns_nr, DDR_REGION_NR_MAX);
		return p;
	}

	/* whole ddr regions boundary, it will be used when parse s-regions */
	p.boundary = max_mb;

	/* calculate ns-region base addr and size offset */
	addr_offset = REGION_ADDR_OFFSET;
	size_offset = REGION_ADDR_OFFSET + p.ns_nr * REGION_DATA_PER_BYTES;

	/* read all ns-regions base and top address */
	for (i = 0; i < p.ns_nr; i++) {
		base = mmio_read_64(addr + addr_offset);
		top = base + mmio_read_64(addr + size_offset);
		/*
		 * translate byte to MB and store info,
		 * Miniloader will promise every ns-region is MB aligned.
		 */
		p.ns_base[i] = RG_SIZE_MB(base);
		p.ns_top[i] = RG_SIZE_MB(top);

		addr_offset += REGION_DATA_PER_BYTES;
		size_offset += REGION_DATA_PER_BYTES;
	}

	/*
	 * a s-region's base starts from previous ns-region's top, and a
	 * s-region's top ends with next ns-region's base. maybe like this:
	 *
	 *	   case1: ns-regison start from 0MB
	 *	 -----------------------------------------------
	 *	 |    ns0   |  S0  |  ns1  |   S1  |    ns2    |
	 *	0----------------------------------------------- max_mb
	 *
	 *
	 *	   case2: ns-regison not start from 0MB
	 *	 -----------------------------------------------
	 *	 |    S0   |  ns0  |  ns1  |   ns2  |    S1    |
	 *	0----------------------------------------------- max_mb
	 */

	/* like above case2 figure, ns-region is not start from 0MB */
	if (p.ns_base[0] != 0) {
		p.s_base[p.s_nr] = 0;
		p.s_top[p.s_nr] = p.ns_base[0];
		p.s_nr++;
	}

	/*
	 * notice: if ns-regions not start from 0MB, p.s_nr = 1 now, otherwise 0
	 */
	for (i = 0; i < p.ns_nr; i++) {
		/*
		 * if current ns-regions top covers boundary,
		 * that means s-regions are all parsed yet, so finsh.
		 */
		if (p.ns_top[i] == p.boundary)
			goto out;

		/* s-region's base starts from previous ns-region's top */
		p.s_base[p.s_nr] = p.ns_top[i];

		/* s-region's top ends with next ns-region's base */
		if (i + 1 < p.ns_nr)
			p.s_top[p.s_nr] = p.ns_base[i + 1];
		else
			p.s_top[p.s_nr] = p.boundary;
		p.s_nr++;
	}
out:
	return p;
}
