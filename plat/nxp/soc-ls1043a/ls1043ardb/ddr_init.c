/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>

#include <errata.h>
#include <platform_def.h>

#ifdef CONFIG_STATIC_DDR
const struct ddr_cfg_regs static_1600 = {
	.cs[0].config = U(0x80040322),
	.cs[0].bnds = U(0x7F),
	.sdram_cfg[0] = U(0xC50C0000),
	.sdram_cfg[1] = U(0x401100),
	.timing_cfg[0] = U(0x91550018),
	.timing_cfg[1] = U(0xBBB48C42),
	.timing_cfg[2] = U(0x48C111),
	.timing_cfg[3] = U(0x10C1000),
	.timing_cfg[4] = U(0x2),
	.timing_cfg[5] = U(0x3401400),
	.timing_cfg[7] = U(0x13300000),
	.timing_cfg[8] = U(0x2115600),
	.sdram_mode[0] = U(0x3010210),
	.sdram_mode[9] = U(0x4000000),
	.sdram_mode[8] = U(0x500),
	.sdram_mode[2] = U(0x10210),
	.sdram_mode[10] = U(0x400),
	.sdram_mode[11] = U(0x4000000),
	.sdram_mode[4] = U(0x10210),
	.sdram_mode[12] = U(0x400),
	.sdram_mode[13] = U(0x4000000),
	.sdram_mode[6] = U(0x10210),
	.sdram_mode[14] = U(0x400),
	.sdram_mode[15] = U(0x4000000),
	.interval = U(0x18600618),
	.zq_cntl = U(0x8A090705),
	.clk_cntl = U(0x3000000),
	.cdr[0] = U(0x80040000),
	.cdr[1] = U(0xA181),
	.wrlvl_cntl[0] = U(0x8675F607),
	.wrlvl_cntl[1] = U(0x7090807,
	.wrlvl_cntl[2] = U(0x7070707),
	.debug[28] = U(0x00700046),
};

uint64_t board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_1600, sizeof(static_1600));

	return ULL(0x80000000);
}

#else
static const struct rc_timing rcz[] = {
	{1600, 12, 7},
	{}
};

static const struct board_timing ram[] = {
	{0x1f, rcz, 0x00020100, 0},
};

int ddr_board_options(struct ddr_info *priv)
{
	int ret;
	struct memctl_opt *popts = &priv->opt;

	ret = cal_board_params(priv, ram, ARRAY_SIZE(ram));
	if (ret)
		return ret;

	popts->cpo_sample = U(0x46);
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN |
			  DDR_CDR1_ODT(DDR_CDR_ODT_80ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_80ohm) |
			  DDR_CDR2_VREF_OVRD(70);	/* Vref = 70% */

	return 0;
}

/* DDR model number: MT40A512M8HX-093E */
struct dimm_params ddr_raw_timing = {
	.n_ranks = U(1),
	.rank_density = ULL(2147483648),
	.capacity = ULL(2147483648),
	.primary_sdram_width = U(32),
	.n_row_addr = U(15),
	.n_col_addr = U(10),
	.bank_group_bits = U(2),
	.burst_lengths_bitmask = U(0x0c),
	.tckmin_x_ps = 938,
	.tckmax_ps = 1500,
	.caslat_x = U(0x000DFA00),
	.taa_ps = 13500,
	.trcd_ps = 13500,
	.trp_ps = 13500,
	.tras_ps = 33000,
	.trc_ps = 46500,
	.twr_ps = 15000,
	.trfc1_ps = 260000,
	.trfc2_ps = 160000,
	.trfc4_ps = 110000,
	.tfaw_ps = 21000,
	.trrds_ps = 3700,
	.trrdl_ps = 5300,
	.tccdl_ps = 5355,
	.refresh_rate_ps = U(7800000),
	.rc = U(0x1f),
};

int ddr_get_ddr_params(struct dimm_params *pdimm,
			    struct ddr_conf *conf)
{
	static const char dimm_model[] = "Fixed DDR on board";

	conf->dimm_in_use[0] = 1;
	memcpy(pdimm, &ddr_raw_timing, sizeof(struct dimm_params));
	memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);

	return 1;
}
#endif

int64_t init_ddr(void)
{
	struct ddr_info info;
	struct sysinfo sys;
	int64_t dram_size;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(struct ddr_info));
	info.num_ctlrs = 1;
	info.dimm_on_ctlr = 1;
	info.clk = get_ddr_freq(&sys, 0);
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);

	if (dram_size < 0) {
		ERROR("DDR init failed\n");
	}

#ifdef ERRATA_SOC_A008850
	erratum_a008850_post();
#endif
	return dram_size;
}
