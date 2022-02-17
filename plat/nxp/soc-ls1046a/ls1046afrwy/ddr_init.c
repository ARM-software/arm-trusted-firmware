/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <ddr.h>
#include <lib/utils.h>

#include <errata.h>
#include <platform_def.h>

#ifdef CONFIG_STATIC_DDR
const struct ddr_cfg_regs static_1600 = {
	.cs[0].config = U(0x80010412),
	.cs[0].bnds = U(0x7F),
	.sdram_cfg[0] = U(0xE50C0008),
	.sdram_cfg[1] = U(0x00401010),
	.sdram_cfg[2] = U(0x1),
	.timing_cfg[0] = U(0xFA550018),
	.timing_cfg[1] = U(0xBAB40C52),
	.timing_cfg[2] = U(0x0048C11C),
	.timing_cfg[3] = U(0x01111000),
	.timing_cfg[4] = U(0x00000002),
	.timing_cfg[5] = U(0x03401400),
	.timing_cfg[6] = U(0x0),
	.timing_cfg[7] = U(0x23300000),
	.timing_cfg[8] = U(0x02116600),
	.timing_cfg[9] = U(0x0),
	.dq_map[0] = U(0x0),
	.dq_map[1] = U(0x0),
	.dq_map[2] = U(0x0),
	.dq_map[3] = U(0x0),
	.sdram_mode[0] = U(0x01010210),
	.sdram_mode[1] = U(0x0),
	.sdram_mode[8] = U(0x00000500),
	.sdram_mode[9] = U(0x04000000),
	.interval = U(0x18600618),
	.zq_cntl = U(0x8A090705),
	.ddr_sr_cntr = U(0x0),
	.clk_cntl = U(0x2000000),
	.cdr[0] = U(0x80040000),
	.cdr[1] = U(0xC1),
	.wrlvl_cntl[0] = U(0x86550607),
	.wrlvl_cntl[1] = U(0x07070708),
	.wrlvl_cntl[2] = U(0x0808088),
};

long long board_static_ddr(struct ddr_info *priv)
{
	memcpy(&priv->ddr_reg, &static_1600, sizeof(static_1600));

	return 0x80000000ULL;
}
#else /* ifndef CONFIG_STATIC_DDR */
static const struct rc_timing rcz[] = {
	{U(1600), U(8), U(7)},
	{U(2100), U(8), U(7)},
	{}
};

static const struct board_timing ram[] = {
	{U(0x1f), rcz, U(0x01010101), U(0x01010101)},
};

int ddr_board_options(struct ddr_info *priv)
{
	int ret;
	struct memctl_opt *popts = &priv->opt;

	ret = cal_board_params(priv, ram, ARRAY_SIZE(ram));
	if (ret != 0) {
		return ret;
	}

	popts->bstopre = 0;
	popts->half_strength_drive_en = 1;
	popts->cpo_sample = U(0x46);
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_50ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_50ohm) |
				DDR_CDR2_VREF_TRAIN_EN;
	popts->output_driver_impedance = 1;

	return 0;
}

/* DDR model number:  MT40A512M16JY-083E:B */
struct dimm_params ddr_raw_timing = {
		.n_ranks = U(1),
		.rank_density = ULL(4294967296),
		.capacity = ULL(4294967296),
		.primary_sdram_width = U(64),
		.ec_sdram_width = U(8),
		.rdimm = U(0),
		.mirrored_dimm = U(0),
		.n_row_addr = U(16),
		.n_col_addr = U(10),
		.bank_group_bits = U(1),
		.edc_config = U(2),
		.burst_lengths_bitmask = U(0x0c),
		.tckmin_x_ps = 750,
		.tckmax_ps = 1900,
		.caslat_x = U(0x0001FFE00),
		.taa_ps = 13500,
		.trcd_ps = 13500,
		.trp_ps = 13500,
		.tras_ps = 33000,
		.trc_ps = 46500,
		.twr_ps = 15000,
		.trfc1_ps = 350000,
		.trfc2_ps = 260000,
		.trfc4_ps = 160000,
		.tfaw_ps = 30000,
		.trrds_ps = 5300,
		.trrdl_ps = 6400,
		.tccdl_ps = 5355,
		.refresh_rate_ps = U(7800000),
		.dq_mapping[0] = U(0x0),
		.dq_mapping[1] = U(0x0),
		.dq_mapping[2] = U(0x0),
		.dq_mapping[3] = U(0x0),
		.dq_mapping[4] = U(0x0),
		.dq_mapping_ors = U(0),
		.rc = U(0x1f),
};

int ddr_get_ddr_params(struct dimm_params *pdimm, struct ddr_conf *conf)
{
	static const char dimm_model[] = "Fixed DDR on board";

	conf->dimm_in_use[0] = 1;
	memcpy(pdimm, &ddr_raw_timing, sizeof(struct dimm_params));
	memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);

	return 1;
}
#endif /* ifdef CONFIG_STATIC_DDR */

long long init_ddr(void)
{
	int spd_addr[] = {NXP_SPD_EEPROM0};
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	if (get_clocks(&sys)) {
		ERROR("System clocks are not set\n");
		assert(0);
	}
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL1 %lu\n", sys.freq_ddr_pll0);
	debug("DDR PLL2 %lu\n", sys.freq_ddr_pll1);

	zeromem(&info, sizeof(struct ddr_info));
	info.num_ctlrs = 1;
	info.dimm_on_ctlr = 1;
	info.clk = get_ddr_freq(&sys, 0);
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);
	if (dram_size < 0) {
		ERROR("DDR init failed.\n");
	}

#ifdef ERRATA_SOC_A008850
	erratum_a008850_post();
#endif

	return dram_size;
}
