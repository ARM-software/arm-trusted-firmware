/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <ddr.h>
#include <utils.h>

#include <errata.h>
#include <platform_def.h>

#ifdef CONFIG_STATIC_DDR
#error No static value defined
#endif

static const struct rc_timing rce[] = {
	{U(1600), U(8), U(8)},
	{U(1867), U(8), U(8)},
	{U(2134), U(8), U(9)},
	{}
};

static const struct board_timing udimm[] = {
	{U(0x04), rce, U(0x01020307), U(0x08090b06)},
};

int ddr_board_options(struct ddr_info *priv)
{
	int ret;
	struct memctl_opt *popts = &priv->opt;

	if (popts->rdimm != 0) {
		debug("RDIMM parameters not set.\n");
		return -EINVAL;
	}

	ret = cal_board_params(priv, udimm, ARRAY_SIZE(udimm));
	if (ret != 0) {
		return ret;
	}

	popts->addr_hash = 1;
	popts->cpo_sample = U(0x7b);
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN	|
			  DDR_CDR1_ODT(DDR_CDR_ODT_60ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_60ohm)	|
			  DDR_CDR2_VREF_TRAIN_EN		|
			  DDR_CDR2_VREF_RANGE_2;

	return 0;
}

long long init_ddr(void)
{
	int spd_addr[] = { NXP_SPD_EEPROM0 };
	struct ddr_info info;
	struct sysinfo sys;
	long long dram_size;

	zeromem(&sys, sizeof(sys));
	get_clocks(&sys);
	debug("platform clock %lu\n", sys.freq_platform);
	debug("DDR PLL %lu\n", sys.freq_ddr_pll0);

	zeromem(&info, sizeof(struct ddr_info));
	info.num_ctlrs = NUM_OF_DDRC;
	info.dimm_on_ctlr = DDRC_NUM_DIMM;
	info.clk = get_ddr_freq(&sys, 0);
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);
	if (dram_size < 0) {
		ERROR("DDR init failed.\n");
	}

	erratum_a008850_post();

	return dram_size;
}
