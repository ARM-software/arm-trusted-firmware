/*
 * Copyright 2019-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <dram.h>

struct dram_info dram_info;

/* Restore the ddrc configs */
void dram_umctl2_init(struct dram_timing_info *timing)
{
	struct dram_cfg_param *ddrc_cfg = timing->ddrc_cfg;
	unsigned int i;

	for (i = 0U; i < timing->ddrc_cfg_num; i++) {
		mmio_write_32(ddrc_cfg->reg, ddrc_cfg->val);
		ddrc_cfg++;
	}

	/* set the default fsp to P0 */
	mmio_write_32(DDRC_MSTR2(0), 0x0);
}

/* Restore the dram PHY config */
void dram_phy_init(struct dram_timing_info *timing)
{
	struct dram_cfg_param *cfg = timing->ddrphy_cfg;
	unsigned int i;

	/* Restore the PHY init config */
	cfg = timing->ddrphy_cfg;
	for (i = 0U; i < timing->ddrphy_cfg_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}

	/* Restore the DDR PHY CSRs */
	cfg = timing->ddrphy_trained_csr;
	for (i = 0U; i < timing->ddrphy_trained_csr_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}

	/* Load the PIE image */
	cfg = timing->ddrphy_pie;
	for (i = 0U; i < timing->ddrphy_pie_num; i++) {
		dwc_ddrphy_apb_wr(cfg->reg, cfg->val);
		cfg++;
	}
}

void dram_info_init(unsigned long dram_timing_base)
{
	uint32_t ddrc_mstr, current_fsp;

	/* Get the dram type & rank */
	ddrc_mstr = mmio_read_32(DDRC_MSTR(0));

	dram_info.dram_type = ddrc_mstr & DDR_TYPE_MASK;
	dram_info.num_rank = (ddrc_mstr >> 24) & ACTIVE_RANK_MASK;

	/* Get current fsp info */
	current_fsp = mmio_read_32(DDRC_DFIMISC(0)) & 0xf;
	dram_info.boot_fsp = current_fsp;
	dram_info.current_fsp = current_fsp;

	dram_info.timing_info = (struct dram_timing_info *)dram_timing_base;
}
