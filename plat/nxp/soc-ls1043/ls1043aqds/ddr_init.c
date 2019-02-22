/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>

#include <common/debug.h>
#include <errno.h>
#include <lib/utils.h>
#include <lib/utils_def.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <ddr.h>
#include <io.h>
#include <errata.h>

static const struct rc_timing rce[] = {
	{1600, 8, 7},
	{1867, 8, 7},
	{2134, 8, 9},
	{}
};

static const struct rc_timing rcd[] = {
	{1600, 8, 6},
	{1867, 8, 9},
	{2134, 8, 10},
	{}
};

static const struct board_timing udimm[] = {
	{0x03, rcd, 0x01020304, 0x06070805},
	{0x04, rce, 0x01020304, 0x06070805},
};

int ddr_board_options(struct ddr_info *priv)
{
	int ret;
	struct memctl_opt *popts = &priv->opt;

	if (popts->rdimm) {
		ERROR("RDIMM parameters not set.\n");
		return -EINVAL;
	}

	ret = cal_board_params(priv, udimm, ARRAY_SIZE(udimm));
	if (ret)
		return ret;

	/* force DDR bus width to 32 bits */
	popts->data_bus_used = DDR_DBUS_32;
	popts->otf_burst_chop_en = 0;
	popts->burst_length = DDR_BL8;
	popts->bstopre = 0;	/* auto precharge */

	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;      /* 15 clocks */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN       |
			  DDR_CDR1_ODT(DDR_CDR_ODT_80ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_80ohm)       |
			  DDR_CDR2_VREF_OVRD(70);

	/* optimize cpo for erratum A-009942 */
	popts->cpo_sample = 0x59;

	return 0;
}

long long _init_ddr(void)
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
	info.num_ctlrs = 1;
	info.dimm_on_ctlr = 1;
	info.clk = get_ddr_freq(&sys, 0);
	info.spd_addr = spd_addr;
	info.ddr[0] = (void *)NXP_DDR_ADDR;

	dram_size = dram_init(&info);

	if (dram_size < 0)
		ERROR("DDR init failed\n");

	erratum_a008850_post();

	return dram_size;
}
