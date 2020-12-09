/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>
#include <load_img.h>

#include "plat_common.h"
#include <platform_def.h>

#ifdef CONFIG_STATIC_DDR
const struct ddr_cfg_regs static_1600 = {
	.cs[0].config = U(0xA8050322),
	.cs[1].config = U(0x80000322),
	.cs[0].bnds = U(0x3FF),
	.cs[1].bnds = U(0x3FF),
	.sdram_cfg[0] = U(0xE5044000),
	.sdram_cfg[1] = U(0x401011),
	.timing_cfg[0] = U(0xFF550018),
	.timing_cfg[1] = U(0xBAB48C42),
	.timing_cfg[2] = U(0x48C111),
	.timing_cfg[3] = U(0x10C1000),
	.timing_cfg[4] = U(0x2),
	.timing_cfg[5] = U(0x3401400),
	.timing_cfg[7] = U(0x13300000),
	.timing_cfg[8] = U(0x2114600),
	.sdram_mode[0] = U(0x6010210),
	.sdram_mode[8] = U(0x500),
	.sdram_mode[9] = U(0x4240000),
	.interval = U(0x18600000),
	.data_init = U(0xDEADBEEF),
	.zq_cntl = U(0x8A090705),
};

const struct dimm_params static_dimm = {
	.rdimm = U(0),
	.primary_sdram_width = U(64),
	.ec_sdram_width = U(8),
	.n_ranks = U(2),
	.device_width = U(8),
	.mirrored_dimm = U(1),
};

/* Sample code using two UDIMM MT18ASF1G72AZ-2G6B1, on each DDR controller */
unsigned long long board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_1600, sizeof(static_1600));
	memcpy(&priv->dimm, &static_dimm, sizeof(static_dimm));
	priv->conf.cs_on_dimm[0] = 0x3;
	ddr_board_options(priv);
	compute_ddr_phy(priv);

	return ULL(0x400000000);
}

#elif defined(CONFIG_DDR_NODIMM)
/*
 * Sample code to bypass reading SPD. This is a sample, not recommended
 * for boards with slots. DDR model number: UDIMM MT18ASF1G72AZ-2G6B1.
 */

const struct dimm_params ddr_raw_timing = {
	.n_ranks = U(2),
	.rank_density = U(4294967296u),
	.capacity = U(8589934592u),
	.primary_sdram_width = U(64),
	.ec_sdram_width = U(8),
	.device_width = U(8),
	.die_density = U(0x4),
	.rdimm = U(0),
	.mirrored_dimm = U(1),
	.n_row_addr = U(15),
	.n_col_addr = U(10),
	.bank_addr_bits = U(0),
	.bank_group_bits = U(2),
	.edc_config = U(2),
	.burst_lengths_bitmask = U(0x0c),
	.tckmin_x_ps = 750,
	.tckmax_ps = 1600,
	.caslat_x = U(0x00FFFC00),
	.taa_ps = 13750,
	.trcd_ps = 13750,
	.trp_ps = 13750,
	.tras_ps = 32000,
	.trc_ps = 457500,
	.twr_ps = 15000,
	.trfc1_ps = 260000,
	.trfc2_ps = 160000,
	.trfc4_ps = 110000,
	.tfaw_ps = 21000,
	.trrds_ps = 3000,
	.trrdl_ps = 4900,
	.tccdl_ps = 5000,
	.refresh_rate_ps = U(7800000),
};

int ddr_get_ddr_params(struct dimm_params *pdimm,
			    struct ddr_conf *conf)
{
	static const char dimm_model[] = "Fixed DDR on board";

	conf->dimm_in_use[0] = 1;	/* Modify accordingly */
	memcpy(pdimm, &ddr_raw_timing, sizeof(struct dimm_params));
	memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);

	/* valid DIMM mask, change accordingly, together with dimm_on_ctlr. */
	return 0x5;
}
#endif	/* CONFIG_DDR_NODIMM */

int ddr_board_options(struct ddr_info *priv)
{
	struct memctl_opt *popts = &priv->opt;
	const struct ddr_conf *conf = &priv->conf;

	popts->vref_dimm = U(0x24);		/* range 1, 83.4% */
	popts->rtt_override = 0;
	popts->rtt_park = U(240);
	popts->otf_burst_chop_en = 0;
	popts->burst_length = U(DDR_BL8);
	popts->trwt_override = U(1);
	popts->bstopre = U(0);			/* auto precharge */
	popts->addr_hash = 1;

	/* Set ODT impedance on PHY side */
	switch (conf->cs_on_dimm[1]) {
	case 0xc:	/* Two slots dual rank */
	case 0x4:	/* Two slots single rank, not valid for interleaving */
		popts->trwt = U(0xf);
		popts->twrt = U(0x7);
		popts->trrt = U(0x7);
		popts->twwt = U(0x7);
		popts->vref_phy = U(0x6B);	/* 83.6% */
		popts->odt = U(60);
		popts->phy_tx_impedance = U(28);
		break;
	case 0:		/* One slot used */
	default:
		popts->trwt = U(0x3);
		popts->twrt = U(0x3);
		popts->trrt = U(0x3);
		popts->twwt = U(0x3);
		popts->vref_phy = U(0x60);	/* 75% */
		popts->odt = U(48);
		popts->phy_tx_impedance = U(28);
		break;
	}

	return 0;
}

long long init_ddr(void)
{
	int spd_addr[] = { 0x51, 0x52, 0x53, 0x54 };
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	if (get_clocks(&sys) != 0) {
		ERROR("System clocks are not set\n");
		panic();
	}
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(info));

	/* Set two DDRC. Unused DDRC will be removed automatically. */
	info.num_ctlrs = NUM_OF_DDRC;
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;
	info.ddr[1] = (void *)NXP_DDR2_ADDR;
	info.phy[0] = (void *)NXP_DDR_PHY1_ADDR;
	info.phy[1] = (void *)NXP_DDR_PHY2_ADDR;
	info.clk = get_ddr_freq(&sys, 0);
	info.img_loadr = load_img;
	info.phy_gen2_fw_img_buf = PHY_GEN2_FW_IMAGE_BUFFER;
	if (info.clk == 0) {
		info.clk = get_ddr_freq(&sys, 1);
	}
	info.dimm_on_ctlr = DDRC_NUM_DIMM;

	info.warm_boot_flag = DDR_WRM_BOOT_NT_SUPPORTED;

	dram_size = dram_init(&info
#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
		    , NXP_CCN_HN_F_0_ADDR
#endif
		    );


	if (dram_size < 0) {
		ERROR("DDR init failed.\n");
	}

	return dram_size;
}
