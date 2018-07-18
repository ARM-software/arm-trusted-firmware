/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <platform_def.h>
#include <thunder_private.h>
#include <thunder_common.h>
#include <thunder_dt.h>
#include <string.h>
#include <debug.h>

static inline uint32_t popcnt(uint64_t val)
{
	uint64_t x, x2 = val;

	x2 = x2 - ((x2 >> 1) & 0x5555555555555555ull);
	/* Every 2 bits holds the sum of every pair of bits (32) */
	x2 = ((x2 >> 2) & 0x3333333333333333ull) + (x2 & 0x3333333333333333ull);
	/* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) (16) */
	x2 = (x2 + (x2 >> 4)) & 0x0F0F0F0F0F0F0F0Full;
	/* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) (8) */

	x = x2 * 0x0101010101010101ull >> 56;

	/* The lower 8 bits hold the popcnt */
	return x;  /* (7 significant bits) */
}

int thunder_dram_is_lmc_enabled(unsigned node, unsigned lmc)
{
	union cavm_lmcx_ddr_pll_ctl lmcx_ddr_pll_ctl;

	lmcx_ddr_pll_ctl.u = CSR_READ_PA(node, CAVM_LMCX_DDR_PLL_CTL(lmc));

	return lmcx_ddr_pll_ctl.s.reset_n;
}

uint64_t thunder_dram_size_node(unsigned node)
{
	uint64_t rank_size, memsize = 0;
	int num_ranks, lmc;
	int lmc_per_node;
	union cavm_lmcx_config lmcx_config;

	/* Fix for ASIM */
	if (!strcmp(bfdt.board_model, "asim-cn81xx"))
		return 0x80000000; /* 2G for T81 */
	else if (!strcmp(bfdt.board_model, "asim-cn83xx"))
		return 0x40000000; /* 1G for T83 */

	lmc_per_node = thunder_get_lmc_per_node();
	if (lmc_per_node < 0) {
		printf("Cannot obtain lmc_per_node count\n");
		return 0;
	}

	if (node >= thunder_get_node_count())
		return 0;

	for (lmc = 0; lmc < lmc_per_node; lmc++) {
		if (!thunder_dram_is_lmc_enabled(node, lmc))
			continue;

		lmcx_config.u = CSR_READ(CSR_PA(node, CAVM_LMCX_PF_BAR0(lmc)), CAVM_LMCX_CONFIG(lmc));
		num_ranks = popcnt(lmcx_config.s.init_status);
		rank_size = 1ull << (28 + lmcx_config.s.pbank_lsb - lmcx_config.s.rank_ena);
		memsize += rank_size * num_ranks;
	}

	/* Safenet for ASIM without configured DRAM size */
	if (memsize == 0) {
		ERROR("DRAM size for ASIM-platform not configured\n");
		memsize = 4ull << 32;
	}

	return memsize;
}
