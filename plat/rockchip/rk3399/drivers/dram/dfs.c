/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <m0_ctl.h>
#include <plat_private.h>
#include "dfs.h"
#include "dram.h"
#include "dram_spec_timing.h"
#include "pmu.h"
#include "soc.h"
#include "string.h"

#define ENPER_CS_TRAINING_FREQ	(666)
#define TDFI_LAT_THRESHOLD_FREQ	(928)
#define PHY_DLL_BYPASS_FREQ	(260)

static const struct pll_div dpll_rates_table[] = {

	/* _mhz, _refdiv, _fbdiv, _postdiv1, _postdiv2 */
	{.mhz = 928, .refdiv = 1, .fbdiv = 116, .postdiv1 = 3, .postdiv2 = 1},
	{.mhz = 800, .refdiv = 1, .fbdiv = 100, .postdiv1 = 3, .postdiv2 = 1},
	{.mhz = 732, .refdiv = 1, .fbdiv = 61, .postdiv1 = 2, .postdiv2 = 1},
	{.mhz = 666, .refdiv = 1, .fbdiv = 111, .postdiv1 = 4, .postdiv2 = 1},
	{.mhz = 600, .refdiv = 1, .fbdiv = 50, .postdiv1 = 2, .postdiv2 = 1},
	{.mhz = 528, .refdiv = 1, .fbdiv = 66, .postdiv1 = 3, .postdiv2 = 1},
	{.mhz = 400, .refdiv = 1, .fbdiv = 50, .postdiv1 = 3, .postdiv2 = 1},
	{.mhz = 300, .refdiv = 1, .fbdiv = 50, .postdiv1 = 4, .postdiv2 = 1},
	{.mhz = 200, .refdiv = 1, .fbdiv = 50, .postdiv1 = 3, .postdiv2 = 2},
};

struct rk3399_dram_status {
	uint32_t current_index;
	uint32_t index_freq[2];
	uint32_t boot_freq;
	uint32_t low_power_stat;
	struct timing_related_config timing_config;
	struct drv_odt_lp_config drv_odt_lp_cfg;
};

struct rk3399_saved_status {
	uint32_t freq;
	uint32_t low_power_stat;
	uint32_t odt;
};

static struct rk3399_dram_status rk3399_dram_status;
static struct rk3399_saved_status rk3399_suspend_status;
static uint32_t wrdqs_delay_val[2][2][4];
static uint32_t rddqs_delay_ps;

static struct rk3399_sdram_default_config ddr3_default_config = {
	.bl = 8,
	.ap = 0,
	.burst_ref_cnt = 1,
	.zqcsi = 0
};

static struct rk3399_sdram_default_config lpddr3_default_config = {
	.bl = 8,
	.ap = 0,
	.burst_ref_cnt = 1,
	.zqcsi = 0
};

static struct rk3399_sdram_default_config lpddr4_default_config = {
	.bl = 16,
	.ap = 0,
	.caodt = 240,
	.burst_ref_cnt = 1,
	.zqcsi = 0
};

static uint32_t get_cs_die_capability(struct rk3399_sdram_params *sdram_config,
		uint8_t channel, uint8_t cs)
{
	struct rk3399_sdram_channel *ch = &sdram_config->ch[channel];
	uint32_t bandwidth;
	uint32_t die_bandwidth;
	uint32_t die;
	uint32_t cs_cap;
	uint32_t row;

	row = cs == 0 ? ch->cs0_row : ch->cs1_row;
	bandwidth = 8 * (1 << ch->bw);
	die_bandwidth = 8 * (1 << ch->dbw);
	die = bandwidth / die_bandwidth;
	cs_cap = (1 << (row + ((1 << ch->bk) / 4 + 1) + ch->col +
		  (bandwidth / 16)));
	if (ch->row_3_4)
		cs_cap = cs_cap * 3 / 4;

	return (cs_cap / die);
}

static void get_dram_drv_odt_val(uint32_t dram_type,
				struct drv_odt_lp_config *drv_config)
{
	uint32_t tmp;
	uint32_t mr1_val, mr3_val, mr11_val;

	switch (dram_type) {
	case DDR3:
		mr1_val = (mmio_read_32(CTL_REG(0, 133)) >> 16) & 0xffff;
		tmp = ((mr1_val >> 1) & 1) | ((mr1_val >> 4) & 1);
		if (tmp)
			drv_config->dram_side_drv = 34;
		else
			drv_config->dram_side_drv = 40;
		tmp = ((mr1_val >> 2) & 1) | ((mr1_val >> 5) & 1) |
		      ((mr1_val >> 7) & 1);
		if (tmp == 0)
			drv_config->dram_side_dq_odt = 0;
		else if (tmp == 1)
			drv_config->dram_side_dq_odt = 60;
		else if (tmp == 3)
			drv_config->dram_side_dq_odt = 40;
		else
			drv_config->dram_side_dq_odt = 120;
		break;
	case LPDDR3:
		mr3_val = mmio_read_32(CTL_REG(0, 138)) & 0xf;
		mr11_val = (mmio_read_32(CTL_REG(0, 139)) >> 24) & 0x3;
		if (mr3_val == 0xb)
			drv_config->dram_side_drv = 3448;
		else if (mr3_val == 0xa)
			drv_config->dram_side_drv = 4048;
		else if (mr3_val == 0x9)
			drv_config->dram_side_drv = 3440;
		else if (mr3_val == 0x4)
			drv_config->dram_side_drv = 60;
		else if (mr3_val == 0x3)
			drv_config->dram_side_drv = 48;
		else if (mr3_val == 0x2)
			drv_config->dram_side_drv = 40;
		else
			drv_config->dram_side_drv = 34;

		if (mr11_val == 1)
			drv_config->dram_side_dq_odt = 60;
		else if (mr11_val == 2)
			drv_config->dram_side_dq_odt = 120;
		else if (mr11_val == 0)
			drv_config->dram_side_dq_odt = 0;
		else
			drv_config->dram_side_dq_odt = 240;
		break;
	case LPDDR4:
	default:
		mr3_val = (mmio_read_32(CTL_REG(0, 138)) >> 3) & 0x7;
		mr11_val = (mmio_read_32(CTL_REG(0, 139)) >> 24) & 0xff;

		if ((mr3_val == 0) || (mr3_val == 7))
			drv_config->dram_side_drv = 40;
		else
			drv_config->dram_side_drv = 240 / mr3_val;

		tmp = mr11_val & 0x7;
		if ((tmp == 7) || (tmp == 0))
			drv_config->dram_side_dq_odt = 0;
		else
			drv_config->dram_side_dq_odt = 240 / tmp;

		tmp = (mr11_val >> 4) & 0x7;
		if ((tmp == 7) || (tmp == 0))
			drv_config->dram_side_ca_odt = 0;
		else
			drv_config->dram_side_ca_odt = 240 / tmp;
		break;
	}
}

static void sdram_timing_cfg_init(struct timing_related_config *ptiming_config,
				  struct rk3399_sdram_params *sdram_params,
				  struct drv_odt_lp_config *drv_config)
{
	uint32_t i, j;

	for (i = 0; i < sdram_params->num_channels; i++) {
		ptiming_config->dram_info[i].speed_rate = DDR3_DEFAULT;
		ptiming_config->dram_info[i].cs_cnt = sdram_params->ch[i].rank;
		for (j = 0; j < sdram_params->ch[i].rank; j++) {
			ptiming_config->dram_info[i].per_die_capability[j] =
			    get_cs_die_capability(sdram_params, i, j);
		}
	}
	ptiming_config->dram_type = sdram_params->dramtype;
	ptiming_config->ch_cnt = sdram_params->num_channels;
	switch (sdram_params->dramtype) {
	case DDR3:
		ptiming_config->bl = ddr3_default_config.bl;
		ptiming_config->ap = ddr3_default_config.ap;
		break;
	case LPDDR3:
		ptiming_config->bl = lpddr3_default_config.bl;
		ptiming_config->ap = lpddr3_default_config.ap;
		break;
	case LPDDR4:
		ptiming_config->bl = lpddr4_default_config.bl;
		ptiming_config->ap = lpddr4_default_config.ap;
		ptiming_config->rdbi = 0;
		ptiming_config->wdbi = 0;
		break;
	default:
		/* Do nothing in default case */
		break;
	}
	ptiming_config->dramds = drv_config->dram_side_drv;
	ptiming_config->dramodt = drv_config->dram_side_dq_odt;
	ptiming_config->caodt = drv_config->dram_side_ca_odt;
	ptiming_config->odt = (mmio_read_32(PHY_REG(0, 5)) >> 16) & 0x1;
}

struct lat_adj_pair {
	uint32_t cl;
	uint32_t rdlat_adj;
	uint32_t cwl;
	uint32_t wrlat_adj;
};

const struct lat_adj_pair ddr3_lat_adj[] = {
	{6, 5, 5, 4},
	{8, 7, 6, 5},
	{10, 9, 7, 6},
	{11, 9, 8, 7},
	{13, 0xb, 9, 8},
	{14, 0xb, 0xa, 9}
};

const struct lat_adj_pair lpddr3_lat_adj[] = {
	{3, 2, 1, 0},
	{6, 5, 3, 2},
	{8, 7, 4, 3},
	{9, 8, 5, 4},
	{10, 9, 6, 5},
	{11, 9, 6, 5},
	{12, 0xa, 6, 5},
	{14, 0xc, 8, 7},
	{16, 0xd, 8, 7}
};

const struct lat_adj_pair lpddr4_lat_adj[] = {
	{6, 5, 4, 2},
	{10, 9, 6, 4},
	{14, 0xc, 8, 6},
	{20, 0x11, 0xa, 8},
	{24, 0x15, 0xc, 0xa},
	{28, 0x18, 0xe, 0xc},
	{32, 0x1b, 0x10, 0xe},
	{36, 0x1e, 0x12, 0x10}
};

static uint32_t get_rdlat_adj(uint32_t dram_type, uint32_t cl)
{
	const struct lat_adj_pair *p;
	uint32_t cnt;
	uint32_t i;

	if (dram_type == DDR3) {
		p = ddr3_lat_adj;
		cnt = ARRAY_SIZE(ddr3_lat_adj);
	} else if (dram_type == LPDDR3) {
		p = lpddr3_lat_adj;
		cnt = ARRAY_SIZE(lpddr3_lat_adj);
	} else {
		p = lpddr4_lat_adj;
		cnt = ARRAY_SIZE(lpddr4_lat_adj);
	}

	for (i = 0; i < cnt; i++) {
		if (cl == p[i].cl)
			return p[i].rdlat_adj;
	}
	/* fail */
	return 0xff;
}

static uint32_t get_wrlat_adj(uint32_t dram_type, uint32_t cwl)
{
	const struct lat_adj_pair *p;
	uint32_t cnt;
	uint32_t i;

	if (dram_type == DDR3) {
		p = ddr3_lat_adj;
		cnt = ARRAY_SIZE(ddr3_lat_adj);
	} else if (dram_type == LPDDR3) {
		p = lpddr3_lat_adj;
		cnt = ARRAY_SIZE(lpddr3_lat_adj);
	} else {
		p = lpddr4_lat_adj;
		cnt = ARRAY_SIZE(lpddr4_lat_adj);
	}

	for (i = 0; i < cnt; i++) {
		if (cwl == p[i].cwl)
			return p[i].wrlat_adj;
	}
	/* fail */
	return 0xff;
}

#define PI_REGS_DIMM_SUPPORT	(0)
#define PI_ADD_LATENCY	(0)
#define PI_DOUBLEFREEK	(1)

#define PI_PAD_DELAY_PS_VALUE	(1000)
#define PI_IE_ENABLE_VALUE	(3000)
#define PI_TSEL_ENABLE_VALUE	(700)

static uint32_t get_pi_rdlat_adj(struct dram_timing_t *pdram_timing)
{
	/*[DLLSUBTYPE2] == "STD_DENALI_HS" */
	uint32_t rdlat, delay_adder, ie_enable, hs_offset, tsel_adder,
	    extra_adder, tsel_enable;

	ie_enable = PI_IE_ENABLE_VALUE;
	tsel_enable = PI_TSEL_ENABLE_VALUE;

	rdlat = pdram_timing->cl + PI_ADD_LATENCY;
	delay_adder = ie_enable / (1000000 / pdram_timing->mhz);
	if ((ie_enable % (1000000 / pdram_timing->mhz)) != 0)
		delay_adder++;
	hs_offset = 0;
	tsel_adder = 0;
	extra_adder = 0;
	/* rdlat = rdlat - (PREAMBLE_SUPPORT & 0x1); */
	tsel_adder = tsel_enable / (1000000 / pdram_timing->mhz);
	if ((tsel_enable % (1000000 / pdram_timing->mhz)) != 0)
		tsel_adder++;
	delay_adder = delay_adder - 1;
	if (tsel_adder > delay_adder)
		extra_adder = tsel_adder - delay_adder;
	else
		extra_adder = 0;
	if (PI_REGS_DIMM_SUPPORT && PI_DOUBLEFREEK)
		hs_offset = 2;
	else
		hs_offset = 1;

	if (delay_adder > (rdlat - 1 - hs_offset)) {
		rdlat = rdlat - tsel_adder;
	} else {
		if ((rdlat - delay_adder) < 2)
			rdlat = 2;
		else
			rdlat = rdlat - delay_adder - extra_adder;
	}

	return rdlat;
}

static uint32_t get_pi_wrlat(struct dram_timing_t *pdram_timing,
			     struct timing_related_config *timing_config)
{
	uint32_t tmp;

	if (timing_config->dram_type == LPDDR3) {
		tmp = pdram_timing->cl;
		if (tmp >= 14)
			tmp = 8;
		else if (tmp >= 10)
			tmp = 6;
		else if (tmp == 9)
			tmp = 5;
		else if (tmp == 8)
			tmp = 4;
		else if (tmp == 6)
			tmp = 3;
		else
			tmp = 1;
	} else {
		tmp = 1;
	}

	return tmp;
}

static uint32_t get_pi_wrlat_adj(struct dram_timing_t *pdram_timing,
				 struct timing_related_config *timing_config)
{
	return get_pi_wrlat(pdram_timing, timing_config) + PI_ADD_LATENCY - 1;
}

static uint32_t get_pi_tdfi_phy_rdlat(struct dram_timing_t *pdram_timing,
			struct timing_related_config *timing_config)
{
	/* [DLLSUBTYPE2] == "STD_DENALI_HS" */
	uint32_t cas_lat, delay_adder, ie_enable, hs_offset, ie_delay_adder;
	uint32_t mem_delay_ps, round_trip_ps;
	uint32_t phy_internal_delay, lpddr_adder, dfi_adder, rdlat_delay;

	ie_enable = PI_IE_ENABLE_VALUE;

	delay_adder = ie_enable / (1000000 / pdram_timing->mhz);
	if ((ie_enable % (1000000 / pdram_timing->mhz)) != 0)
		delay_adder++;
	delay_adder = delay_adder - 1;
	if (PI_REGS_DIMM_SUPPORT && PI_DOUBLEFREEK)
		hs_offset = 2;
	else
		hs_offset = 1;

	cas_lat = pdram_timing->cl + PI_ADD_LATENCY;

	if (delay_adder > (cas_lat - 1 - hs_offset)) {
		ie_delay_adder = 0;
	} else {
		ie_delay_adder = ie_enable / (1000000 / pdram_timing->mhz);
		if ((ie_enable % (1000000 / pdram_timing->mhz)) != 0)
			ie_delay_adder++;
	}

	if (timing_config->dram_type == DDR3) {
		mem_delay_ps = 0;
	} else if (timing_config->dram_type == LPDDR4) {
		mem_delay_ps = 3600;
	} else if (timing_config->dram_type == LPDDR3) {
		mem_delay_ps = 5500;
	} else {
		NOTICE("get_pi_tdfi_phy_rdlat:dramtype unsupport\n");
		return 0;
	}
	round_trip_ps = 1100 + 500 + mem_delay_ps + 500 + 600;
	delay_adder = round_trip_ps / (1000000 / pdram_timing->mhz);
	if ((round_trip_ps % (1000000 / pdram_timing->mhz)) != 0)
		delay_adder++;

	phy_internal_delay = 5 + 2 + 4;
	lpddr_adder = mem_delay_ps / (1000000 / pdram_timing->mhz);
	if ((mem_delay_ps % (1000000 / pdram_timing->mhz)) != 0)
		lpddr_adder++;
	dfi_adder = 0;
	phy_internal_delay = phy_internal_delay + 2;
	rdlat_delay = delay_adder + phy_internal_delay +
	    ie_delay_adder + lpddr_adder + dfi_adder;

	rdlat_delay = rdlat_delay + 2;
	return rdlat_delay;
}

static uint32_t get_pi_todtoff_min(struct dram_timing_t *pdram_timing,
				   struct timing_related_config *timing_config)
{
	uint32_t tmp, todtoff_min_ps;

	if (timing_config->dram_type == LPDDR3)
		todtoff_min_ps = 2500;
	else if (timing_config->dram_type == LPDDR4)
		todtoff_min_ps = 1500;
	else
		todtoff_min_ps = 0;
	/* todtoff_min */
	tmp = todtoff_min_ps / (1000000 / pdram_timing->mhz);
	if ((todtoff_min_ps % (1000000 / pdram_timing->mhz)) != 0)
		tmp++;
	return tmp;
}

static uint32_t get_pi_todtoff_max(struct dram_timing_t *pdram_timing,
				   struct timing_related_config *timing_config)
{
	uint32_t tmp, todtoff_max_ps;

	if ((timing_config->dram_type == LPDDR4)
	    || (timing_config->dram_type == LPDDR3))
		todtoff_max_ps = 3500;
	else
		todtoff_max_ps = 0;

	/* todtoff_max */
	tmp = todtoff_max_ps / (1000000 / pdram_timing->mhz);
	if ((todtoff_max_ps % (1000000 / pdram_timing->mhz)) != 0)
		tmp++;
	return tmp;
}

static void gen_rk3399_ctl_params_f0(struct timing_related_config
				     *timing_config,
				     struct dram_timing_t *pdram_timing)
{
	uint32_t i;
	uint32_t tmp, tmp1;

	for (i = 0; i < timing_config->ch_cnt; i++) {
		if (timing_config->dram_type == DDR3) {
			tmp = ((700000 + 10) * timing_config->freq +
				999) / 1000;
			tmp += pdram_timing->txsnr + (pdram_timing->tmrd * 3) +
			    pdram_timing->tmod + pdram_timing->tzqinit;
			mmio_write_32(CTL_REG(i, 5), tmp);

			mmio_clrsetbits_32(CTL_REG(i, 22), 0xffff,
					   pdram_timing->tdllk);

			mmio_write_32(CTL_REG(i, 32),
				      (pdram_timing->tmod << 8) |
				       pdram_timing->tmrd);

			mmio_clrsetbits_32(CTL_REG(i, 59), 0xffffu << 16,
					   (pdram_timing->txsr -
					    pdram_timing->trcd) << 16);
		} else if (timing_config->dram_type == LPDDR4) {
			mmio_write_32(CTL_REG(i, 5), pdram_timing->tinit1 +
						     pdram_timing->tinit3);
			mmio_write_32(CTL_REG(i, 32),
				      (pdram_timing->tmrd << 8) |
				      pdram_timing->tmrd);
			mmio_clrsetbits_32(CTL_REG(i, 59), 0xffffu << 16,
					   pdram_timing->txsr << 16);
		} else {
			mmio_write_32(CTL_REG(i, 5), pdram_timing->tinit1);
			mmio_write_32(CTL_REG(i, 7), pdram_timing->tinit4);
			mmio_write_32(CTL_REG(i, 32),
				      (pdram_timing->tmrd << 8) |
				      pdram_timing->tmrd);
			mmio_clrsetbits_32(CTL_REG(i, 59), 0xffffu << 16,
					   pdram_timing->txsr << 16);
		}
		mmio_write_32(CTL_REG(i, 6), pdram_timing->tinit3);
		mmio_write_32(CTL_REG(i, 8), pdram_timing->tinit5);
		mmio_clrsetbits_32(CTL_REG(i, 23), (0x7f << 16),
				   ((pdram_timing->cl * 2) << 16));
		mmio_clrsetbits_32(CTL_REG(i, 23), (0x1f << 24),
				   (pdram_timing->cwl << 24));
		mmio_clrsetbits_32(CTL_REG(i, 24), 0x3f, pdram_timing->al);
		mmio_clrsetbits_32(CTL_REG(i, 26), 0xffffu << 16,
				   (pdram_timing->trc << 24) |
				   (pdram_timing->trrd << 16));
		mmio_write_32(CTL_REG(i, 27),
			      (pdram_timing->tfaw << 24) |
			      (pdram_timing->trppb << 16) |
			      (pdram_timing->twtr << 8) |
			      pdram_timing->tras_min);

		mmio_clrsetbits_32(CTL_REG(i, 31), 0xffu << 24,
				   max(4, pdram_timing->trtp) << 24);
		mmio_write_32(CTL_REG(i, 33), (pdram_timing->tcke << 24) |
					      pdram_timing->tras_max);
		mmio_clrsetbits_32(CTL_REG(i, 34), 0xff,
				   max(1, pdram_timing->tckesr));
		mmio_clrsetbits_32(CTL_REG(i, 39),
				   (0x3f << 16) | (0xff << 8),
				   (pdram_timing->twr << 16) |
				   (pdram_timing->trcd << 8));
		mmio_clrsetbits_32(CTL_REG(i, 42), 0x1f << 16,
				   pdram_timing->tmrz << 16);
		tmp = pdram_timing->tdal ? pdram_timing->tdal :
		      (pdram_timing->twr + pdram_timing->trp);
		mmio_clrsetbits_32(CTL_REG(i, 44), 0xff, tmp);
		mmio_clrsetbits_32(CTL_REG(i, 45), 0xff, pdram_timing->trp);
		mmio_write_32(CTL_REG(i, 48),
			      ((pdram_timing->trefi - 8) << 16) |
			      pdram_timing->trfc);
		mmio_clrsetbits_32(CTL_REG(i, 52), 0xffff, pdram_timing->txp);
		mmio_clrsetbits_32(CTL_REG(i, 53), 0xffffu << 16,
				   pdram_timing->txpdll << 16);
		mmio_clrsetbits_32(CTL_REG(i, 55), 0xf << 24,
				   pdram_timing->tcscke << 24);
		mmio_clrsetbits_32(CTL_REG(i, 55), 0xff, pdram_timing->tmrri);
		mmio_write_32(CTL_REG(i, 56),
			      (pdram_timing->tzqcke << 24) |
			      (pdram_timing->tmrwckel << 16) |
			      (pdram_timing->tckehcs << 8) |
			      pdram_timing->tckelcs);
		mmio_clrsetbits_32(CTL_REG(i, 60), 0xffff, pdram_timing->txsnr);
		mmio_clrsetbits_32(CTL_REG(i, 62), 0xffffu << 16,
				   (pdram_timing->tckehcmd << 24) |
				   (pdram_timing->tckelcmd << 16));
		mmio_write_32(CTL_REG(i, 63),
			      (pdram_timing->tckelpd << 24) |
			      (pdram_timing->tescke << 16) |
			      (pdram_timing->tsr << 8) |
			      pdram_timing->tckckel);
		mmio_clrsetbits_32(CTL_REG(i, 64), 0xfff,
				   (pdram_timing->tcmdcke << 8) |
				   pdram_timing->tcsckeh);
		mmio_clrsetbits_32(CTL_REG(i, 92), 0xffff << 8,
				   (pdram_timing->tcksrx << 16) |
				   (pdram_timing->tcksre << 8));
		mmio_clrsetbits_32(CTL_REG(i, 108), 0x1 << 24,
				   (timing_config->dllbp << 24));
		mmio_clrsetbits_32(CTL_REG(i, 122), 0x3ff << 16,
				   (pdram_timing->tvrcg_enable << 16));
		mmio_write_32(CTL_REG(i, 123), (pdram_timing->tfc_long << 16) |
					       pdram_timing->tvrcg_disable);
		mmio_write_32(CTL_REG(i, 124),
			      (pdram_timing->tvref_long << 16) |
			      (pdram_timing->tckfspx << 8) |
			      pdram_timing->tckfspe);
		mmio_write_32(CTL_REG(i, 133), (pdram_timing->mr[1] << 16) |
					       pdram_timing->mr[0]);
		mmio_clrsetbits_32(CTL_REG(i, 134), 0xffff,
				   pdram_timing->mr[2]);
		mmio_clrsetbits_32(CTL_REG(i, 138), 0xffff,
				   pdram_timing->mr[3]);
		mmio_clrsetbits_32(CTL_REG(i, 139), 0xffu << 24,
				   pdram_timing->mr11 << 24);
		mmio_write_32(CTL_REG(i, 147),
			      (pdram_timing->mr[1] << 16) |
			      pdram_timing->mr[0]);
		mmio_clrsetbits_32(CTL_REG(i, 148), 0xffff,
				   pdram_timing->mr[2]);
		mmio_clrsetbits_32(CTL_REG(i, 152), 0xffff,
				   pdram_timing->mr[3]);
		mmio_clrsetbits_32(CTL_REG(i, 153), 0xffu << 24,
				   pdram_timing->mr11 << 24);
		if (timing_config->dram_type == LPDDR4) {
			mmio_clrsetbits_32(CTL_REG(i, 140), 0xffffu << 16,
					   pdram_timing->mr12 << 16);
			mmio_clrsetbits_32(CTL_REG(i, 142), 0xffffu << 16,
					   pdram_timing->mr14 << 16);
			mmio_clrsetbits_32(CTL_REG(i, 145), 0xffffu << 16,
					   pdram_timing->mr22 << 16);
			mmio_clrsetbits_32(CTL_REG(i, 154), 0xffffu << 16,
					   pdram_timing->mr12 << 16);
			mmio_clrsetbits_32(CTL_REG(i, 156), 0xffffu << 16,
					   pdram_timing->mr14 << 16);
			mmio_clrsetbits_32(CTL_REG(i, 159), 0xffffu << 16,
					   pdram_timing->mr22 << 16);
		}
		mmio_clrsetbits_32(CTL_REG(i, 179), 0xfff << 8,
				   pdram_timing->tzqinit << 8);
		mmio_write_32(CTL_REG(i, 180), (pdram_timing->tzqcs << 16) |
					       (pdram_timing->tzqinit / 2));
		mmio_write_32(CTL_REG(i, 181), (pdram_timing->tzqlat << 16) |
					       pdram_timing->tzqcal);
		mmio_clrsetbits_32(CTL_REG(i, 212), 0xff << 8,
				   pdram_timing->todton << 8);

		if (timing_config->odt) {
			mmio_setbits_32(CTL_REG(i, 213), 1 << 16);
			if (timing_config->freq < 400)
				tmp = 4 << 24;
			else
				tmp = 8 << 24;
		} else {
			mmio_clrbits_32(CTL_REG(i, 213), 1 << 16);
			tmp = 2 << 24;
		}

		mmio_clrsetbits_32(CTL_REG(i, 216), 0x1f << 24, tmp);
		mmio_clrsetbits_32(CTL_REG(i, 221), (0x3 << 16) | (0xf << 8),
				   (pdram_timing->tdqsck << 16) |
				   (pdram_timing->tdqsck_max << 8));
		tmp =
		    (get_wrlat_adj(timing_config->dram_type, pdram_timing->cwl)
		     << 8) | get_rdlat_adj(timing_config->dram_type,
					   pdram_timing->cl);
		mmio_clrsetbits_32(CTL_REG(i, 284), 0xffff, tmp);
		mmio_clrsetbits_32(CTL_REG(i, 82), 0xffffu << 16,
				   (4 * pdram_timing->trefi) << 16);

		mmio_clrsetbits_32(CTL_REG(i, 83), 0xffff,
				   (2 * pdram_timing->trefi) & 0xffff);

		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			tmp = get_pi_wrlat(pdram_timing, timing_config);
			tmp1 = get_pi_todtoff_max(pdram_timing, timing_config);
			tmp = (tmp > tmp1) ? (tmp - tmp1) : 0;
		} else {
			tmp = 0;
		}
		mmio_clrsetbits_32(CTL_REG(i, 214), 0x3f << 16,
				   (tmp & 0x3f) << 16);

		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			/* min_rl_preamble = cl+TDQSCK_MIN -1 */
			tmp = pdram_timing->cl +
			    get_pi_todtoff_min(pdram_timing, timing_config) - 1;
			/* todtoff_max */
			tmp1 = get_pi_todtoff_max(pdram_timing, timing_config);
			tmp = (tmp > tmp1) ? (tmp - tmp1) : 0;
		} else {
			tmp = pdram_timing->cl - pdram_timing->cwl;
		}
		mmio_clrsetbits_32(CTL_REG(i, 215), 0x3f << 8,
				   (tmp & 0x3f) << 8);

		mmio_clrsetbits_32(CTL_REG(i, 275), 0xff << 16,
				   (get_pi_tdfi_phy_rdlat(pdram_timing,
							  timing_config) &
				    0xff) << 16);

		mmio_clrsetbits_32(CTL_REG(i, 277), 0xffff,
				   (2 * pdram_timing->trefi) & 0xffff);

		mmio_clrsetbits_32(CTL_REG(i, 282), 0xffff,
				   (2 * pdram_timing->trefi) & 0xffff);

		mmio_write_32(CTL_REG(i, 283), 20 * pdram_timing->trefi);

		/* CTL_308 TDFI_CALVL_CAPTURE_F0:RW:16:10 */
		tmp1 = 20000 / (1000000 / pdram_timing->mhz) + 1;
		if ((20000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		tmp = (tmp1 >> 1) + (tmp1 % 2) + 5;
		mmio_clrsetbits_32(CTL_REG(i, 308), 0x3ff << 16, tmp << 16);

		/* CTL_308 TDFI_CALVL_CC_F0:RW:0:10 */
		tmp = tmp + 18;
		mmio_clrsetbits_32(CTL_REG(i, 308), 0x3ff, tmp);

		/* CTL_314 TDFI_WRCSLAT_F0:RW:8:8 */
		tmp1 = get_pi_wrlat_adj(pdram_timing, timing_config);
		if (timing_config->freq <= TDFI_LAT_THRESHOLD_FREQ) {
			if (tmp1 == 0)
				tmp = 0;
			else if (tmp1 < 5)
				tmp = tmp1 - 1;
			else
				tmp = tmp1 - 5;
		} else {
			tmp = tmp1 - 2;
		}
		mmio_clrsetbits_32(CTL_REG(i, 314), 0xff << 8, tmp << 8);

		/* CTL_314 TDFI_RDCSLAT_F0:RW:0:8 */
		if ((timing_config->freq <= TDFI_LAT_THRESHOLD_FREQ) &&
		    (pdram_timing->cl >= 5))
			tmp = pdram_timing->cl - 5;
		else
			tmp = pdram_timing->cl - 2;
		mmio_clrsetbits_32(CTL_REG(i, 314), 0xff, tmp);
	}
}

static void gen_rk3399_ctl_params_f1(struct timing_related_config
				     *timing_config,
				     struct dram_timing_t *pdram_timing)
{
	uint32_t i;
	uint32_t tmp, tmp1;

	for (i = 0; i < timing_config->ch_cnt; i++) {
		if (timing_config->dram_type == DDR3) {
			tmp =
			    ((700000 + 10) * timing_config->freq + 999) / 1000;
			tmp += pdram_timing->txsnr + (pdram_timing->tmrd * 3) +
			       pdram_timing->tmod + pdram_timing->tzqinit;
			mmio_write_32(CTL_REG(i, 9), tmp);
			mmio_clrsetbits_32(CTL_REG(i, 22), 0xffffu << 16,
					   pdram_timing->tdllk << 16);
			mmio_clrsetbits_32(CTL_REG(i, 34), 0xffffff00,
					   (pdram_timing->tmod << 24) |
					   (pdram_timing->tmrd << 16) |
					   (pdram_timing->trtp << 8));
			mmio_clrsetbits_32(CTL_REG(i, 60), 0xffffu << 16,
					   (pdram_timing->txsr -
					    pdram_timing->trcd) << 16);
		} else if (timing_config->dram_type == LPDDR4) {
			mmio_write_32(CTL_REG(i, 9), pdram_timing->tinit1 +
						     pdram_timing->tinit3);
			mmio_clrsetbits_32(CTL_REG(i, 34), 0xffffff00,
					   (pdram_timing->tmrd << 24) |
					   (pdram_timing->tmrd << 16) |
					   (pdram_timing->trtp << 8));
			mmio_clrsetbits_32(CTL_REG(i, 60), 0xffffu << 16,
					   pdram_timing->txsr << 16);
		} else {
			mmio_write_32(CTL_REG(i, 9), pdram_timing->tinit1);
			mmio_write_32(CTL_REG(i, 11), pdram_timing->tinit4);
			mmio_clrsetbits_32(CTL_REG(i, 34), 0xffffff00,
					   (pdram_timing->tmrd << 24) |
					   (pdram_timing->tmrd << 16) |
					   (pdram_timing->trtp << 8));
			mmio_clrsetbits_32(CTL_REG(i, 60), 0xffffu << 16,
					   pdram_timing->txsr << 16);
		}
		mmio_write_32(CTL_REG(i, 10), pdram_timing->tinit3);
		mmio_write_32(CTL_REG(i, 12), pdram_timing->tinit5);
		mmio_clrsetbits_32(CTL_REG(i, 24), (0x7f << 8),
				   ((pdram_timing->cl * 2) << 8));
		mmio_clrsetbits_32(CTL_REG(i, 24), (0x1f << 16),
				   (pdram_timing->cwl << 16));
		mmio_clrsetbits_32(CTL_REG(i, 24), 0x3f << 24,
				   pdram_timing->al << 24);
		mmio_clrsetbits_32(CTL_REG(i, 28), 0xffffff00,
				   (pdram_timing->tras_min << 24) |
				   (pdram_timing->trc << 16) |
				   (pdram_timing->trrd << 8));
		mmio_clrsetbits_32(CTL_REG(i, 29), 0xffffff,
				   (pdram_timing->tfaw << 16) |
				   (pdram_timing->trppb << 8) |
				   pdram_timing->twtr);
		mmio_write_32(CTL_REG(i, 35), (pdram_timing->tcke << 24) |
					      pdram_timing->tras_max);
		mmio_clrsetbits_32(CTL_REG(i, 36), 0xff,
				   max(1, pdram_timing->tckesr));
		mmio_clrsetbits_32(CTL_REG(i, 39), (0xffu << 24),
				   (pdram_timing->trcd << 24));
		mmio_clrsetbits_32(CTL_REG(i, 40), 0x3f, pdram_timing->twr);
		mmio_clrsetbits_32(CTL_REG(i, 42), 0x1f << 24,
				   pdram_timing->tmrz << 24);
		tmp = pdram_timing->tdal ? pdram_timing->tdal :
		      (pdram_timing->twr + pdram_timing->trp);
		mmio_clrsetbits_32(CTL_REG(i, 44), 0xff << 8, tmp << 8);
		mmio_clrsetbits_32(CTL_REG(i, 45), 0xff << 8,
				   pdram_timing->trp << 8);
		mmio_write_32(CTL_REG(i, 49),
			      ((pdram_timing->trefi - 8) << 16) |
			      pdram_timing->trfc);
		mmio_clrsetbits_32(CTL_REG(i, 52), 0xffffu << 16,
				   pdram_timing->txp << 16);
		mmio_clrsetbits_32(CTL_REG(i, 54), 0xffff,
				   pdram_timing->txpdll);
		mmio_clrsetbits_32(CTL_REG(i, 55), 0xff << 8,
				   pdram_timing->tmrri << 8);
		mmio_write_32(CTL_REG(i, 57), (pdram_timing->tmrwckel << 24) |
					      (pdram_timing->tckehcs << 16) |
					      (pdram_timing->tckelcs << 8) |
					      pdram_timing->tcscke);
		mmio_clrsetbits_32(CTL_REG(i, 58), 0xf, pdram_timing->tzqcke);
		mmio_clrsetbits_32(CTL_REG(i, 61), 0xffff, pdram_timing->txsnr);
		mmio_clrsetbits_32(CTL_REG(i, 64), 0xffffu << 16,
				   (pdram_timing->tckehcmd << 24) |
				   (pdram_timing->tckelcmd << 16));
		mmio_write_32(CTL_REG(i, 65), (pdram_timing->tckelpd << 24) |
					      (pdram_timing->tescke << 16) |
					      (pdram_timing->tsr << 8) |
					      pdram_timing->tckckel);
		mmio_clrsetbits_32(CTL_REG(i, 66), 0xfff,
				   (pdram_timing->tcmdcke << 8) |
				   pdram_timing->tcsckeh);
		mmio_clrsetbits_32(CTL_REG(i, 92), (0xffu << 24),
				   (pdram_timing->tcksre << 24));
		mmio_clrsetbits_32(CTL_REG(i, 93), 0xff,
				   pdram_timing->tcksrx);
		mmio_clrsetbits_32(CTL_REG(i, 108), (0x1 << 25),
				   (timing_config->dllbp << 25));
		mmio_write_32(CTL_REG(i, 125),
			      (pdram_timing->tvrcg_disable << 16) |
			      pdram_timing->tvrcg_enable);
		mmio_write_32(CTL_REG(i, 126), (pdram_timing->tckfspx << 24) |
					       (pdram_timing->tckfspe << 16) |
					       pdram_timing->tfc_long);
		mmio_clrsetbits_32(CTL_REG(i, 127), 0xffff,
				   pdram_timing->tvref_long);
		mmio_clrsetbits_32(CTL_REG(i, 134), 0xffffu << 16,
				   pdram_timing->mr[0] << 16);
		mmio_write_32(CTL_REG(i, 135), (pdram_timing->mr[2] << 16) |
					       pdram_timing->mr[1]);
		mmio_clrsetbits_32(CTL_REG(i, 138), 0xffffu << 16,
				   pdram_timing->mr[3] << 16);
		mmio_clrsetbits_32(CTL_REG(i, 140), 0xff, pdram_timing->mr11);
		mmio_clrsetbits_32(CTL_REG(i, 148), 0xffffu << 16,
				   pdram_timing->mr[0] << 16);
		mmio_write_32(CTL_REG(i, 149), (pdram_timing->mr[2] << 16) |
					       pdram_timing->mr[1]);
		mmio_clrsetbits_32(CTL_REG(i, 152), 0xffffu << 16,
				   pdram_timing->mr[3] << 16);
		mmio_clrsetbits_32(CTL_REG(i, 154), 0xff, pdram_timing->mr11);
		if (timing_config->dram_type == LPDDR4) {
			mmio_clrsetbits_32(CTL_REG(i, 141), 0xffff,
					   pdram_timing->mr12);
			mmio_clrsetbits_32(CTL_REG(i, 143), 0xffff,
					   pdram_timing->mr14);
			mmio_clrsetbits_32(CTL_REG(i, 146), 0xffff,
					   pdram_timing->mr22);
			mmio_clrsetbits_32(CTL_REG(i, 155), 0xffff,
					   pdram_timing->mr12);
			mmio_clrsetbits_32(CTL_REG(i, 157), 0xffff,
					   pdram_timing->mr14);
			mmio_clrsetbits_32(CTL_REG(i, 160), 0xffff,
					   pdram_timing->mr22);
		}
		mmio_write_32(CTL_REG(i, 182),
			      ((pdram_timing->tzqinit / 2) << 16) |
			      pdram_timing->tzqinit);
		mmio_write_32(CTL_REG(i, 183), (pdram_timing->tzqcal << 16) |
					       pdram_timing->tzqcs);
		mmio_clrsetbits_32(CTL_REG(i, 184), 0x3f, pdram_timing->tzqlat);
		mmio_clrsetbits_32(CTL_REG(i, 188), 0xfff,
				   pdram_timing->tzqreset);
		mmio_clrsetbits_32(CTL_REG(i, 212), 0xff << 16,
				   pdram_timing->todton << 16);

		if (timing_config->odt) {
			mmio_setbits_32(CTL_REG(i, 213), (1 << 24));
			if (timing_config->freq < 400)
				tmp = 4 << 24;
			else
				tmp = 8 << 24;
		} else {
			mmio_clrbits_32(CTL_REG(i, 213), (1 << 24));
			tmp = 2 << 24;
		}
		mmio_clrsetbits_32(CTL_REG(i, 217), 0x1f << 24, tmp);
		mmio_clrsetbits_32(CTL_REG(i, 221), 0xf << 24,
				   (pdram_timing->tdqsck_max << 24));
		mmio_clrsetbits_32(CTL_REG(i, 222), 0x3, pdram_timing->tdqsck);
		mmio_clrsetbits_32(CTL_REG(i, 291), 0xffff,
				   (get_wrlat_adj(timing_config->dram_type,
						  pdram_timing->cwl) << 8) |
				   get_rdlat_adj(timing_config->dram_type,
						 pdram_timing->cl));

		mmio_clrsetbits_32(CTL_REG(i, 84), 0xffff,
				   (4 * pdram_timing->trefi) & 0xffff);

		mmio_clrsetbits_32(CTL_REG(i, 84), 0xffffu << 16,
				   ((2 * pdram_timing->trefi) & 0xffff) << 16);

		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			tmp = get_pi_wrlat(pdram_timing, timing_config);
			tmp1 = get_pi_todtoff_max(pdram_timing, timing_config);
			tmp = (tmp > tmp1) ? (tmp - tmp1) : 0;
		} else {
			tmp = 0;
		}
		mmio_clrsetbits_32(CTL_REG(i, 214), 0x3f << 24,
				   (tmp & 0x3f) << 24);

		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			/* min_rl_preamble = cl + TDQSCK_MIN - 1 */
			tmp = pdram_timing->cl +
			      get_pi_todtoff_min(pdram_timing, timing_config);
			tmp--;
			/* todtoff_max */
			tmp1 = get_pi_todtoff_max(pdram_timing, timing_config);
			tmp = (tmp > tmp1) ? (tmp - tmp1) : 0;
		} else {
			tmp = pdram_timing->cl - pdram_timing->cwl;
		}
		mmio_clrsetbits_32(CTL_REG(i, 215), 0x3f << 16,
				   (tmp & 0x3f) << 16);

		mmio_clrsetbits_32(CTL_REG(i, 275), 0xffu << 24,
				   (get_pi_tdfi_phy_rdlat(pdram_timing,
							  timing_config) &
				    0xff) << 24);

		mmio_clrsetbits_32(CTL_REG(i, 284), 0xffffu << 16,
				   ((2 * pdram_timing->trefi) & 0xffff) << 16);

		mmio_clrsetbits_32(CTL_REG(i, 289), 0xffff,
				   (2 * pdram_timing->trefi) & 0xffff);

		mmio_write_32(CTL_REG(i, 290), 20 * pdram_timing->trefi);

		/* CTL_309 TDFI_CALVL_CAPTURE_F1:RW:16:10 */
		tmp1 = 20000 / (1000000 / pdram_timing->mhz) + 1;
		if ((20000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		tmp = (tmp1 >> 1) + (tmp1 % 2) + 5;
		mmio_clrsetbits_32(CTL_REG(i, 309), 0x3ff << 16, tmp << 16);

		/* CTL_309 TDFI_CALVL_CC_F1:RW:0:10 */
		tmp = tmp + 18;
		mmio_clrsetbits_32(CTL_REG(i, 309), 0x3ff, tmp);

		/* CTL_314 TDFI_WRCSLAT_F1:RW:24:8 */
		tmp1 = get_pi_wrlat_adj(pdram_timing, timing_config);
		if (timing_config->freq <= TDFI_LAT_THRESHOLD_FREQ) {
			if (tmp1 == 0)
				tmp = 0;
			else if (tmp1 < 5)
				tmp = tmp1 - 1;
			else
				tmp = tmp1 - 5;
		} else {
			tmp = tmp1 - 2;
		}

		mmio_clrsetbits_32(CTL_REG(i, 314), 0xffu << 24, tmp << 24);

		/* CTL_314 TDFI_RDCSLAT_F1:RW:16:8 */
		if ((timing_config->freq <= TDFI_LAT_THRESHOLD_FREQ) &&
		    (pdram_timing->cl >= 5))
			tmp = pdram_timing->cl - 5;
		else
			tmp = pdram_timing->cl - 2;
		mmio_clrsetbits_32(CTL_REG(i, 314), 0xff << 16, tmp << 16);
	}
}

static void gen_rk3399_enable_training(uint32_t ch_cnt, uint32_t nmhz)
{
		uint32_t i, tmp;

		if (nmhz <= PHY_DLL_BYPASS_FREQ)
			tmp = 0;
		else
			tmp = 1;

		for (i = 0; i < ch_cnt; i++) {
			mmio_clrsetbits_32(CTL_REG(i, 305), 1 << 16, tmp << 16);
			mmio_clrsetbits_32(CTL_REG(i, 71), 1, tmp);
			mmio_clrsetbits_32(CTL_REG(i, 70), 1 << 8, 1 << 8);
		}
}

static void gen_rk3399_disable_training(uint32_t ch_cnt)
{
	uint32_t i;

	for (i = 0; i < ch_cnt; i++) {
		mmio_clrbits_32(CTL_REG(i, 305), 1 << 16);
		mmio_clrbits_32(CTL_REG(i, 71), 1);
		mmio_clrbits_32(CTL_REG(i, 70), 1 << 8);
	}
}

static void gen_rk3399_ctl_params(struct timing_related_config *timing_config,
				  struct dram_timing_t *pdram_timing,
				  uint32_t fn)
{
	if (fn == 0)
		gen_rk3399_ctl_params_f0(timing_config, pdram_timing);
	else
		gen_rk3399_ctl_params_f1(timing_config, pdram_timing);
}

static void gen_rk3399_pi_params_f0(struct timing_related_config *timing_config,
				    struct dram_timing_t *pdram_timing)
{
	uint32_t tmp, tmp1, tmp2;
	uint32_t i;

	for (i = 0; i < timing_config->ch_cnt; i++) {
		/* PI_02 PI_TDFI_PHYMSTR_MAX_F0:RW:0:32 */
		tmp = 4 * pdram_timing->trefi;
		mmio_write_32(PI_REG(i, 2), tmp);
		/* PI_03 PI_TDFI_PHYMSTR_RESP_F0:RW:0:16 */
		tmp = 2 * pdram_timing->trefi;
		mmio_clrsetbits_32(PI_REG(i, 3), 0xffff, tmp);
		/* PI_07 PI_TDFI_PHYUPD_RESP_F0:RW:16:16 */
		mmio_clrsetbits_32(PI_REG(i, 7), 0xffffu << 16, tmp << 16);

		/* PI_42 PI_TDELAY_RDWR_2_BUS_IDLE_F0:RW:0:8 */
		if (timing_config->dram_type == LPDDR4)
			tmp = 2;
		else
			tmp = 0;
		tmp = (pdram_timing->bl / 2) + 4 +
		      (get_pi_rdlat_adj(pdram_timing) - 2) + tmp +
		      get_pi_tdfi_phy_rdlat(pdram_timing, timing_config);
		mmio_clrsetbits_32(PI_REG(i, 42), 0xff, tmp);
		/* PI_43 PI_WRLAT_F0:RW:0:5 */
		if (timing_config->dram_type == LPDDR3) {
			tmp = get_pi_wrlat(pdram_timing, timing_config);
			mmio_clrsetbits_32(PI_REG(i, 43), 0x1f, tmp);
		}
		/* PI_43 PI_ADDITIVE_LAT_F0:RW:8:6 */
		mmio_clrsetbits_32(PI_REG(i, 43), 0x3f << 8,
				   PI_ADD_LATENCY << 8);

		/* PI_43 PI_CASLAT_LIN_F0:RW:16:7 */
		mmio_clrsetbits_32(PI_REG(i, 43), 0x7f << 16,
				   (pdram_timing->cl * 2) << 16);
		/* PI_46 PI_TREF_F0:RW:16:16 */
		mmio_clrsetbits_32(PI_REG(i, 46), 0xffffu << 16,
				   pdram_timing->trefi << 16);
		/* PI_46 PI_TRFC_F0:RW:0:10 */
		mmio_clrsetbits_32(PI_REG(i, 46), 0x3ff, pdram_timing->trfc);
		/* PI_66 PI_TODTL_2CMD_F0:RW:24:8 */
		if (timing_config->dram_type == LPDDR3) {
			tmp = get_pi_todtoff_max(pdram_timing, timing_config);
			mmio_clrsetbits_32(PI_REG(i, 66), 0xffu << 24,
					   tmp << 24);
		}
		/* PI_72 PI_WR_TO_ODTH_F0:RW:16:6 */
		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			tmp1 = get_pi_wrlat(pdram_timing, timing_config);
			tmp2 = get_pi_todtoff_max(pdram_timing, timing_config);
			if (tmp1 > tmp2)
				tmp = tmp1 - tmp2;
			else
				tmp = 0;
		} else if (timing_config->dram_type == DDR3) {
			tmp = 0;
		}
		mmio_clrsetbits_32(PI_REG(i, 72), 0x3f << 16, tmp << 16);
		/* PI_73 PI_RD_TO_ODTH_F0:RW:8:6 */
		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			/* min_rl_preamble = cl + TDQSCK_MIN - 1 */
			tmp1 = pdram_timing->cl;
			tmp1 += get_pi_todtoff_min(pdram_timing, timing_config);
			tmp1--;
			/* todtoff_max */
			tmp2 = get_pi_todtoff_max(pdram_timing, timing_config);
			if (tmp1 > tmp2)
				tmp = tmp1 - tmp2;
			else
				tmp = 0;
		} else if (timing_config->dram_type == DDR3) {
			tmp = pdram_timing->cl - pdram_timing->cwl;
		}
		mmio_clrsetbits_32(PI_REG(i, 73), 0x3f << 8, tmp << 8);
		/* PI_89 PI_RDLAT_ADJ_F0:RW:16:8 */
		tmp = get_pi_rdlat_adj(pdram_timing);
		mmio_clrsetbits_32(PI_REG(i, 89), 0xff << 16, tmp << 16);
		/* PI_90 PI_WRLAT_ADJ_F0:RW:16:8 */
		tmp = get_pi_wrlat_adj(pdram_timing, timing_config);
		mmio_clrsetbits_32(PI_REG(i, 90), 0xff << 16, tmp << 16);
		/* PI_91 PI_TDFI_WRCSLAT_F0:RW:16:8 */
		tmp1 = tmp;
		if (tmp1 == 0)
			tmp = 0;
		else if (tmp1 < 5)
			tmp = tmp1 - 1;
		else
			tmp = tmp1 - 5;
		mmio_clrsetbits_32(PI_REG(i, 91), 0xff << 16, tmp << 16);
		/* PI_95 PI_TDFI_CALVL_CAPTURE_F0:RW:16:10 */
		tmp1 = 20000 / (1000000 / pdram_timing->mhz) + 1;
		if ((20000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		tmp = (tmp1 >> 1) + (tmp1 % 2) + 5;
		mmio_clrsetbits_32(PI_REG(i, 95), 0x3ff << 16, tmp << 16);
		/* PI_95 PI_TDFI_CALVL_CC_F0:RW:0:10 */
		mmio_clrsetbits_32(PI_REG(i, 95), 0x3ff, tmp + 18);
		/* PI_102 PI_TMRZ_F0:RW:8:5 */
		mmio_clrsetbits_32(PI_REG(i, 102), 0x1f << 8,
				   pdram_timing->tmrz << 8);
		/* PI_111 PI_TDFI_CALVL_STROBE_F0:RW:8:4 */
		tmp1 = 2 * 1000 / (1000000 / pdram_timing->mhz);
		if ((2 * 1000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		/* pi_tdfi_calvl_strobe=tds_train+5 */
		tmp = tmp1 + 5;
		mmio_clrsetbits_32(PI_REG(i, 111), 0xf << 8, tmp << 8);
		/* PI_116 PI_TCKEHDQS_F0:RW:16:6 */
		tmp = 10000 / (1000000 / pdram_timing->mhz);
		if ((10000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp++;
		if (pdram_timing->mhz <= 100)
			tmp = tmp + 1;
		else
			tmp = tmp + 8;
		mmio_clrsetbits_32(PI_REG(i, 116), 0x3f << 16, tmp << 16);
		/* PI_125 PI_MR1_DATA_F0_0:RW+:8:16 */
		mmio_clrsetbits_32(PI_REG(i, 125), 0xffff << 8,
				   pdram_timing->mr[1] << 8);
		/* PI_133 PI_MR1_DATA_F0_1:RW+:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 133), 0xffff, pdram_timing->mr[1]);
		/* PI_140 PI_MR1_DATA_F0_2:RW+:16:16 */
		mmio_clrsetbits_32(PI_REG(i, 140), 0xffffu << 16,
				   pdram_timing->mr[1] << 16);
		/* PI_148 PI_MR1_DATA_F0_3:RW+:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 148), 0xffff, pdram_timing->mr[1]);
		/* PI_126 PI_MR2_DATA_F0_0:RW+:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 126), 0xffff, pdram_timing->mr[2]);
		/* PI_133 PI_MR2_DATA_F0_1:RW+:16:16 */
		mmio_clrsetbits_32(PI_REG(i, 133), 0xffffu << 16,
				   pdram_timing->mr[2] << 16);
		/* PI_141 PI_MR2_DATA_F0_2:RW+:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 141), 0xffff, pdram_timing->mr[2]);
		/* PI_148 PI_MR2_DATA_F0_3:RW+:16:16 */
		mmio_clrsetbits_32(PI_REG(i, 148), 0xffffu << 16,
				   pdram_timing->mr[2] << 16);
		/* PI_156 PI_TFC_F0:RW:0:10 */
		mmio_clrsetbits_32(PI_REG(i, 156), 0x3ff,
				   pdram_timing->tfc_long);
		/* PI_158 PI_TWR_F0:RW:24:6 */
		mmio_clrsetbits_32(PI_REG(i, 158), 0x3f << 24,
				   pdram_timing->twr << 24);
		/* PI_158 PI_TWTR_F0:RW:16:6 */
		mmio_clrsetbits_32(PI_REG(i, 158), 0x3f << 16,
				   pdram_timing->twtr << 16);
		/* PI_158 PI_TRCD_F0:RW:8:8 */
		mmio_clrsetbits_32(PI_REG(i, 158), 0xff << 8,
				   pdram_timing->trcd << 8);
		/* PI_158 PI_TRP_F0:RW:0:8 */
		mmio_clrsetbits_32(PI_REG(i, 158), 0xff, pdram_timing->trp);
		/* PI_157 PI_TRTP_F0:RW:24:8 */
		mmio_clrsetbits_32(PI_REG(i, 157), 0xffu << 24,
				   pdram_timing->trtp << 24);
		/* PI_159 PI_TRAS_MIN_F0:RW:24:8 */
		mmio_clrsetbits_32(PI_REG(i, 159), 0xffu << 24,
				   pdram_timing->tras_min << 24);
		/* PI_159 PI_TRAS_MAX_F0:RW:0:17 */
		tmp = pdram_timing->tras_max * 99 / 100;
		mmio_clrsetbits_32(PI_REG(i, 159), 0x1ffff, tmp);
		/* PI_160 PI_TMRD_F0:RW:16:6 */
		mmio_clrsetbits_32(PI_REG(i, 160), 0x3f << 16,
				   pdram_timing->tmrd << 16);
		/*PI_160 PI_TDQSCK_MAX_F0:RW:0:4 */
		mmio_clrsetbits_32(PI_REG(i, 160), 0xf,
				   pdram_timing->tdqsck_max);
		/* PI_187 PI_TDFI_CTRLUPD_MAX_F0:RW:8:16 */
		mmio_clrsetbits_32(PI_REG(i, 187), 0xffff << 8,
				   (2 * pdram_timing->trefi) << 8);
		/* PI_188 PI_TDFI_CTRLUPD_INTERVAL_F0:RW:0:32 */
		mmio_clrsetbits_32(PI_REG(i, 188), 0xffffffff,
				   20 * pdram_timing->trefi);
	}
}

static void gen_rk3399_pi_params_f1(struct timing_related_config *timing_config,
				    struct dram_timing_t *pdram_timing)
{
	uint32_t tmp, tmp1, tmp2;
	uint32_t i;

	for (i = 0; i < timing_config->ch_cnt; i++) {
		/* PI_04 PI_TDFI_PHYMSTR_MAX_F1:RW:0:32 */
		tmp = 4 * pdram_timing->trefi;
		mmio_write_32(PI_REG(i, 4), tmp);
		/* PI_05 PI_TDFI_PHYMSTR_RESP_F1:RW:0:16 */
		tmp = 2 * pdram_timing->trefi;
		mmio_clrsetbits_32(PI_REG(i, 5), 0xffff, tmp);
		/* PI_12 PI_TDFI_PHYUPD_RESP_F1:RW:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 12), 0xffff, tmp);

		/* PI_42 PI_TDELAY_RDWR_2_BUS_IDLE_F1:RW:8:8 */
		if (timing_config->dram_type == LPDDR4)
			tmp = 2;
		else
			tmp = 0;
		tmp = (pdram_timing->bl / 2) + 4 +
		      (get_pi_rdlat_adj(pdram_timing) - 2) + tmp +
		      get_pi_tdfi_phy_rdlat(pdram_timing, timing_config);
		mmio_clrsetbits_32(PI_REG(i, 42), 0xff << 8, tmp << 8);
		/* PI_43 PI_WRLAT_F1:RW:24:5 */
		if (timing_config->dram_type == LPDDR3) {
			tmp = get_pi_wrlat(pdram_timing, timing_config);
			mmio_clrsetbits_32(PI_REG(i, 43), 0x1f << 24,
					   tmp << 24);
		}
		/* PI_44 PI_ADDITIVE_LAT_F1:RW:0:6 */
		mmio_clrsetbits_32(PI_REG(i, 44), 0x3f, PI_ADD_LATENCY);
		/* PI_44 PI_CASLAT_LIN_F1:RW:8:7:=0x18 */
		mmio_clrsetbits_32(PI_REG(i, 44), 0x7f << 8,
				   (pdram_timing->cl * 2) << 8);
		/* PI_47 PI_TREF_F1:RW:16:16 */
		mmio_clrsetbits_32(PI_REG(i, 47), 0xffffu << 16,
				   pdram_timing->trefi << 16);
		/* PI_47 PI_TRFC_F1:RW:0:10 */
		mmio_clrsetbits_32(PI_REG(i, 47), 0x3ff, pdram_timing->trfc);
		/* PI_67 PI_TODTL_2CMD_F1:RW:8:8 */
		if (timing_config->dram_type == LPDDR3) {
			tmp = get_pi_todtoff_max(pdram_timing, timing_config);
			mmio_clrsetbits_32(PI_REG(i, 67), 0xff << 8, tmp << 8);
		}
		/* PI_72 PI_WR_TO_ODTH_F1:RW:24:6 */
		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			tmp1 = get_pi_wrlat(pdram_timing, timing_config);
			tmp2 = get_pi_todtoff_max(pdram_timing, timing_config);
			if (tmp1 > tmp2)
				tmp = tmp1 - tmp2;
			else
				tmp = 0;
		} else if (timing_config->dram_type == DDR3) {
			tmp = 0;
		}
		mmio_clrsetbits_32(PI_REG(i, 72), 0x3f << 24, tmp << 24);
		/* PI_73 PI_RD_TO_ODTH_F1:RW:16:6 */
		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			/* min_rl_preamble = cl + TDQSCK_MIN - 1 */
			tmp1 = pdram_timing->cl +
			       get_pi_todtoff_min(pdram_timing, timing_config);
			tmp1--;
			/* todtoff_max */
			tmp2 = get_pi_todtoff_max(pdram_timing, timing_config);
			if (tmp1 > tmp2)
				tmp = tmp1 - tmp2;
			else
				tmp = 0;
		} else if (timing_config->dram_type == DDR3)
			tmp = pdram_timing->cl - pdram_timing->cwl;

		mmio_clrsetbits_32(PI_REG(i, 73), 0x3f << 16, tmp << 16);
		/*P I_89 PI_RDLAT_ADJ_F1:RW:24:8 */
		tmp = get_pi_rdlat_adj(pdram_timing);
		mmio_clrsetbits_32(PI_REG(i, 89), 0xffu << 24, tmp << 24);
		/* PI_90 PI_WRLAT_ADJ_F1:RW:24:8 */
		tmp = get_pi_wrlat_adj(pdram_timing, timing_config);
		mmio_clrsetbits_32(PI_REG(i, 90), 0xffu << 24, tmp << 24);
		/* PI_91 PI_TDFI_WRCSLAT_F1:RW:24:8 */
		tmp1 = tmp;
		if (tmp1 == 0)
			tmp = 0;
		else if (tmp1 < 5)
			tmp = tmp1 - 1;
		else
			tmp = tmp1 - 5;
		mmio_clrsetbits_32(PI_REG(i, 91), 0xffu << 24, tmp << 24);
		/*PI_96 PI_TDFI_CALVL_CAPTURE_F1:RW:16:10 */
		/* tadr=20ns */
		tmp1 = 20000 / (1000000 / pdram_timing->mhz) + 1;
		if ((20000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		tmp = (tmp1 >> 1) + (tmp1 % 2) + 5;
		mmio_clrsetbits_32(PI_REG(i, 96), 0x3ff << 16, tmp << 16);
		/* PI_96 PI_TDFI_CALVL_CC_F1:RW:0:10 */
		tmp = tmp + 18;
		mmio_clrsetbits_32(PI_REG(i, 96), 0x3ff, tmp);
		/*PI_103 PI_TMRZ_F1:RW:0:5 */
		mmio_clrsetbits_32(PI_REG(i, 103), 0x1f, pdram_timing->tmrz);
		/*PI_111 PI_TDFI_CALVL_STROBE_F1:RW:16:4 */
		/* tds_train=ceil(2/ns) */
		tmp1 = 2 * 1000 / (1000000 / pdram_timing->mhz);
		if ((2 * 1000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		/* pi_tdfi_calvl_strobe=tds_train+5 */
		tmp = tmp1 + 5;
		mmio_clrsetbits_32(PI_REG(i, 111), 0xf << 16,
				   tmp << 16);
		/* PI_116 PI_TCKEHDQS_F1:RW:24:6 */
		tmp = 10000 / (1000000 / pdram_timing->mhz);
		if ((10000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp++;
		if (pdram_timing->mhz <= 100)
			tmp = tmp + 1;
		else
			tmp = tmp + 8;
		mmio_clrsetbits_32(PI_REG(i, 116), 0x3f << 24,
				   tmp << 24);
		/* PI_128 PI_MR1_DATA_F1_0:RW+:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 128), 0xffff, pdram_timing->mr[1]);
		/* PI_135 PI_MR1_DATA_F1_1:RW+:8:16 */
		mmio_clrsetbits_32(PI_REG(i, 135), 0xffff << 8,
				   pdram_timing->mr[1] << 8);
		/* PI_143 PI_MR1_DATA_F1_2:RW+:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 143), 0xffff, pdram_timing->mr[1]);
		/* PI_150 PI_MR1_DATA_F1_3:RW+:8:16 */
		mmio_clrsetbits_32(PI_REG(i, 150), 0xffff << 8,
				   pdram_timing->mr[1] << 8);
		/* PI_128 PI_MR2_DATA_F1_0:RW+:16:16 */
		mmio_clrsetbits_32(PI_REG(i, 128), 0xffffu << 16,
				   pdram_timing->mr[2] << 16);
		/* PI_136 PI_MR2_DATA_F1_1:RW+:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 136), 0xffff, pdram_timing->mr[2]);
		/* PI_143 PI_MR2_DATA_F1_2:RW+:16:16 */
		mmio_clrsetbits_32(PI_REG(i, 143), 0xffffu << 16,
				   pdram_timing->mr[2] << 16);
		/* PI_151 PI_MR2_DATA_F1_3:RW+:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 151), 0xffff, pdram_timing->mr[2]);
		/* PI_156 PI_TFC_F1:RW:16:10 */
		mmio_clrsetbits_32(PI_REG(i, 156), 0x3ff << 16,
				   pdram_timing->tfc_long << 16);
		/* PI_162 PI_TWR_F1:RW:8:6 */
		mmio_clrsetbits_32(PI_REG(i, 162), 0x3f << 8,
				   pdram_timing->twr << 8);
		/* PI_162 PI_TWTR_F1:RW:0:6 */
		mmio_clrsetbits_32(PI_REG(i, 162), 0x3f, pdram_timing->twtr);
		/* PI_161 PI_TRCD_F1:RW:24:8 */
		mmio_clrsetbits_32(PI_REG(i, 161), 0xffu << 24,
				   pdram_timing->trcd << 24);
		/* PI_161 PI_TRP_F1:RW:16:8 */
		mmio_clrsetbits_32(PI_REG(i, 161), 0xff << 16,
				   pdram_timing->trp << 16);
		/* PI_161 PI_TRTP_F1:RW:8:8 */
		mmio_clrsetbits_32(PI_REG(i, 161), 0xff << 8,
				   pdram_timing->trtp << 8);
		/* PI_163 PI_TRAS_MIN_F1:RW:24:8 */
		mmio_clrsetbits_32(PI_REG(i, 163), 0xffu << 24,
				   pdram_timing->tras_min << 24);
		/* PI_163 PI_TRAS_MAX_F1:RW:0:17 */
		mmio_clrsetbits_32(PI_REG(i, 163), 0x1ffff,
				   pdram_timing->tras_max * 99 / 100);
		/* PI_164 PI_TMRD_F1:RW:16:6 */
		mmio_clrsetbits_32(PI_REG(i, 164), 0x3f << 16,
				   pdram_timing->tmrd << 16);
		/* PI_164 PI_TDQSCK_MAX_F1:RW:0:4 */
		mmio_clrsetbits_32(PI_REG(i, 164), 0xf,
				   pdram_timing->tdqsck_max);
		/* PI_189 PI_TDFI_CTRLUPD_MAX_F1:RW:0:16 */
		mmio_clrsetbits_32(PI_REG(i, 189), 0xffff,
				   2 * pdram_timing->trefi);
		/* PI_190 PI_TDFI_CTRLUPD_INTERVAL_F1:RW:0:32 */
		mmio_clrsetbits_32(PI_REG(i, 190), 0xffffffff,
				   20 * pdram_timing->trefi);
	}
}

static void gen_rk3399_pi_params(struct timing_related_config *timing_config,
				 struct dram_timing_t *pdram_timing,
				 uint32_t fn)
{
	if (fn == 0)
		gen_rk3399_pi_params_f0(timing_config, pdram_timing);
	else
		gen_rk3399_pi_params_f1(timing_config, pdram_timing);
}

static void gen_rk3399_set_odt(uint32_t odt_en)
{
	uint32_t drv_odt_val;
	uint32_t i;

	for (i = 0; i < rk3399_dram_status.timing_config.ch_cnt; i++) {
		drv_odt_val = (odt_en | (0 << 1) | (0 << 2)) << 16;
		mmio_clrsetbits_32(PHY_REG(i, 5), 0x7 << 16, drv_odt_val);
		mmio_clrsetbits_32(PHY_REG(i, 133), 0x7 << 16, drv_odt_val);
		mmio_clrsetbits_32(PHY_REG(i, 261), 0x7 << 16, drv_odt_val);
		mmio_clrsetbits_32(PHY_REG(i, 389), 0x7 << 16, drv_odt_val);
		drv_odt_val = (odt_en | (0 << 1) | (0 << 2)) << 24;
		mmio_clrsetbits_32(PHY_REG(i, 6), 0x7 << 24, drv_odt_val);
		mmio_clrsetbits_32(PHY_REG(i, 134), 0x7 << 24, drv_odt_val);
		mmio_clrsetbits_32(PHY_REG(i, 262), 0x7 << 24, drv_odt_val);
		mmio_clrsetbits_32(PHY_REG(i, 390), 0x7 << 24, drv_odt_val);
	}
}

static void gen_rk3399_phy_dll_bypass(uint32_t mhz, uint32_t ch,
		uint32_t index, uint32_t dram_type)
{
	uint32_t sw_master_mode = 0;
	uint32_t rddqs_gate_delay, rddqs_latency, total_delay;
	uint32_t i;

	if (dram_type == DDR3)
		total_delay = PI_PAD_DELAY_PS_VALUE;
	else if (dram_type == LPDDR3)
		total_delay = PI_PAD_DELAY_PS_VALUE + 2500;
	else
		total_delay = PI_PAD_DELAY_PS_VALUE + 1500;
	/* total_delay + 0.55tck */
	total_delay +=  (55 * 10000)/mhz;
	rddqs_latency = total_delay * mhz / 1000000;
	total_delay -= rddqs_latency * 1000000 / mhz;
	rddqs_gate_delay = total_delay * 0x200 * mhz / 1000000;
	if (mhz <= PHY_DLL_BYPASS_FREQ) {
		sw_master_mode = 0xc;
		mmio_setbits_32(PHY_REG(ch, 514), 1);
		mmio_setbits_32(PHY_REG(ch, 642), 1);
		mmio_setbits_32(PHY_REG(ch, 770), 1);

		/* setting bypass mode slave delay */
		for (i = 0; i < 4; i++) {
			/* wr dq delay = -180deg + (0x60 / 4) * 20ps */
			mmio_clrsetbits_32(PHY_REG(ch, 1 + 128 * i), 0x7ff << 8,
					   0x4a0 << 8);
			/* rd dqs/dq delay = (0x60 / 4) * 20ps */
			mmio_clrsetbits_32(PHY_REG(ch, 11 + 128 * i), 0x3ff,
					   0xa0);
			/* rd rddqs_gate delay */
			mmio_clrsetbits_32(PHY_REG(ch, 2 + 128 * i), 0x3ff,
					   rddqs_gate_delay);
			mmio_clrsetbits_32(PHY_REG(ch, 78 + 128 * i), 0xf,
					   rddqs_latency);
		}
		for (i = 0; i < 3; i++)
			/* adr delay */
			mmio_clrsetbits_32(PHY_REG(ch, 513 + 128 * i),
					   0x7ff << 16, 0x80 << 16);

		if ((mmio_read_32(PHY_REG(ch, 86)) & 0xc00) == 0) {
			/*
			 * old status is normal mode,
			 * and saving the wrdqs slave delay
			 */
			for (i = 0; i < 4; i++) {
				/* save and clear wr dqs slave delay */
				wrdqs_delay_val[ch][index][i] = 0x3ff &
					(mmio_read_32(PHY_REG(ch, 63 + i * 128))
					>> 16);
				mmio_clrsetbits_32(PHY_REG(ch, 63 + i * 128),
						   0x03ff << 16, 0 << 16);
				/*
				 * in normal mode the cmd may delay 1cycle by
				 * wrlvl and in bypass mode making dqs also
				 * delay 1cycle.
				 */
				mmio_clrsetbits_32(PHY_REG(ch, 78 + i * 128),
						   0x07 << 8, 0x1 << 8);
			}
		}
	} else if (mmio_read_32(PHY_REG(ch, 86)) & 0xc00) {
		/* old status is bypass mode and restore wrlvl resume */
		for (i = 0; i < 4; i++) {
			mmio_clrsetbits_32(PHY_REG(ch, 63 + i * 128),
					   0x03ff << 16,
					   (wrdqs_delay_val[ch][index][i] &
					    0x3ff) << 16);
			/* resume phy_write_path_lat_add */
			mmio_clrbits_32(PHY_REG(ch, 78 + i * 128), 0x07 << 8);
		}
	}

	/* phy_sw_master_mode_X PHY_86/214/342/470 4bits offset_8 */
	mmio_clrsetbits_32(PHY_REG(ch, 86), 0xf << 8, sw_master_mode << 8);
	mmio_clrsetbits_32(PHY_REG(ch, 214), 0xf << 8, sw_master_mode << 8);
	mmio_clrsetbits_32(PHY_REG(ch, 342), 0xf << 8, sw_master_mode << 8);
	mmio_clrsetbits_32(PHY_REG(ch, 470), 0xf << 8, sw_master_mode << 8);

	/* phy_adrctl_sw_master_mode PHY_547/675/803 4bits offset_16 */
	mmio_clrsetbits_32(PHY_REG(ch, 547), 0xf << 16, sw_master_mode << 16);
	mmio_clrsetbits_32(PHY_REG(ch, 675), 0xf << 16, sw_master_mode << 16);
	mmio_clrsetbits_32(PHY_REG(ch, 803), 0xf << 16, sw_master_mode << 16);
}

static void gen_rk3399_phy_params(struct timing_related_config *timing_config,
				  struct drv_odt_lp_config *drv_config,
				  struct dram_timing_t *pdram_timing,
				  uint32_t fn)
{
	uint32_t tmp, i, div, j;
	uint32_t mem_delay_ps, pad_delay_ps, total_delay_ps, delay_frac_ps;
	uint32_t trpre_min_ps, gate_delay_ps, gate_delay_frac_ps;
	uint32_t ie_enable, tsel_enable, cas_lat, rddata_en_ie_dly, tsel_adder;
	uint32_t extra_adder, delta, hs_offset;

	for (i = 0; i < timing_config->ch_cnt; i++) {

		pad_delay_ps = PI_PAD_DELAY_PS_VALUE;
		ie_enable = PI_IE_ENABLE_VALUE;
		tsel_enable = PI_TSEL_ENABLE_VALUE;

		mmio_clrsetbits_32(PHY_REG(i, 896), (0x3 << 8) | 1, fn << 8);

		/* PHY_LOW_FREQ_SEL */
		/* DENALI_PHY_913 1bit offset_0 */
		if (timing_config->freq > 400)
			mmio_clrbits_32(PHY_REG(i, 913), 1);
		else
			mmio_setbits_32(PHY_REG(i, 913), 1);

		/* PHY_RPTR_UPDATE_x */
		/* DENALI_PHY_87/215/343/471 4bit offset_16 */
		tmp = 2500 / (1000000 / pdram_timing->mhz) + 3;
		if ((2500 % (1000000 / pdram_timing->mhz)) != 0)
			tmp++;
		mmio_clrsetbits_32(PHY_REG(i, 87), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 215), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 343), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 471), 0xf << 16, tmp << 16);

		/* PHY_PLL_CTRL */
		/* DENALI_PHY_911 13bits offset_0 */
		/* PHY_LP4_BOOT_PLL_CTRL */
		/* DENALI_PHY_919 13bits offset_0 */
		tmp = (1 << 12) | (2 << 7) | (1 << 1);
		mmio_clrsetbits_32(PHY_REG(i, 911), 0x1fff, tmp);
		mmio_clrsetbits_32(PHY_REG(i, 919), 0x1fff, tmp);

		/* PHY_PLL_CTRL_CA */
		/* DENALI_PHY_911 13bits offset_16 */
		/* PHY_LP4_BOOT_PLL_CTRL_CA */
		/* DENALI_PHY_919 13bits offset_16 */
		tmp = (2 << 7) | (1 << 5) | (1 << 1);
		mmio_clrsetbits_32(PHY_REG(i, 911), 0x1fff << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 919), 0x1fff << 16, tmp << 16);

		/* PHY_TCKSRE_WAIT */
		/* DENALI_PHY_922 4bits offset_24 */
		if (pdram_timing->mhz <= 400)
			tmp = 1;
		else if (pdram_timing->mhz <= 800)
			tmp = 3;
		else if (pdram_timing->mhz <= 1000)
			tmp = 4;
		else
			tmp = 5;
		mmio_clrsetbits_32(PHY_REG(i, 922), 0xf << 24, tmp << 24);
		/* PHY_CAL_CLK_SELECT_0:RW8:3 */
		div = pdram_timing->mhz / (2 * 20);
		for (j = 2, tmp = 1; j <= 128; j <<= 1, tmp++) {
			if (div < j)
				break;
		}
		mmio_clrsetbits_32(PHY_REG(i, 947), 0x7 << 8, tmp << 8);

		if (timing_config->dram_type == DDR3) {
			mem_delay_ps = 0;
			trpre_min_ps = 1000;
		} else if (timing_config->dram_type == LPDDR4) {
			mem_delay_ps = 1500;
			trpre_min_ps = 900;
		} else if (timing_config->dram_type == LPDDR3) {
			mem_delay_ps = 2500;
			trpre_min_ps = 900;
		} else {
			ERROR("gen_rk3399_phy_params:dramtype unsupport\n");
			return;
		}
		total_delay_ps = mem_delay_ps + pad_delay_ps;
		delay_frac_ps = 1000 * total_delay_ps /
				(1000000 / pdram_timing->mhz);
		gate_delay_ps = delay_frac_ps + 1000 - (trpre_min_ps / 2);
		gate_delay_frac_ps = gate_delay_ps % 1000;
		tmp = gate_delay_frac_ps * 0x200 / 1000;
		/* PHY_RDDQS_GATE_SLAVE_DELAY */
		/* DENALI_PHY_77/205/333/461 10bits offset_16 */
		mmio_clrsetbits_32(PHY_REG(i, 77), 0x2ff << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 205), 0x2ff << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 333), 0x2ff << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 461), 0x2ff << 16, tmp << 16);

		tmp = gate_delay_ps / 1000;
		/* PHY_LP4_BOOT_RDDQS_LATENCY_ADJUST */
		/* DENALI_PHY_10/138/266/394 4bit offset_0 */
		mmio_clrsetbits_32(PHY_REG(i, 10), 0xf, tmp);
		mmio_clrsetbits_32(PHY_REG(i, 138), 0xf, tmp);
		mmio_clrsetbits_32(PHY_REG(i, 266), 0xf, tmp);
		mmio_clrsetbits_32(PHY_REG(i, 394), 0xf, tmp);
		/* PHY_GTLVL_LAT_ADJ_START */
		/* DENALI_PHY_80/208/336/464 4bits offset_16 */
		tmp = rddqs_delay_ps / (1000000 / pdram_timing->mhz) + 2;
		mmio_clrsetbits_32(PHY_REG(i, 80), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 208), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 336), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 464), 0xf << 16, tmp << 16);

		cas_lat = pdram_timing->cl + PI_ADD_LATENCY;
		rddata_en_ie_dly = ie_enable / (1000000 / pdram_timing->mhz);
		if ((ie_enable % (1000000 / pdram_timing->mhz)) != 0)
			rddata_en_ie_dly++;
		rddata_en_ie_dly = rddata_en_ie_dly - 1;
		tsel_adder = tsel_enable / (1000000 / pdram_timing->mhz);
		if ((tsel_enable % (1000000 / pdram_timing->mhz)) != 0)
			tsel_adder++;
		if (rddata_en_ie_dly > tsel_adder)
			extra_adder = rddata_en_ie_dly - tsel_adder;
		else
			extra_adder = 0;
		delta = cas_lat - rddata_en_ie_dly;
		if (PI_REGS_DIMM_SUPPORT && PI_DOUBLEFREEK)
			hs_offset = 2;
		else
			hs_offset = 1;
		if (rddata_en_ie_dly > (cas_lat - 1 - hs_offset))
			tmp = 0;
		else if ((delta == 2) || (delta == 1))
			tmp = rddata_en_ie_dly - 0 - extra_adder;
		else
			tmp = extra_adder;
		/* PHY_LP4_BOOT_RDDATA_EN_TSEL_DLY */
		/* DENALI_PHY_9/137/265/393 4bit offset_16 */
		mmio_clrsetbits_32(PHY_REG(i, 9), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 137), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 265), 0xf << 16, tmp << 16);
		mmio_clrsetbits_32(PHY_REG(i, 393), 0xf << 16, tmp << 16);
		/* PHY_RDDATA_EN_TSEL_DLY */
		/* DENALI_PHY_86/214/342/470 4bit offset_0 */
		mmio_clrsetbits_32(PHY_REG(i, 86), 0xf, tmp);
		mmio_clrsetbits_32(PHY_REG(i, 214), 0xf, tmp);
		mmio_clrsetbits_32(PHY_REG(i, 342), 0xf, tmp);
		mmio_clrsetbits_32(PHY_REG(i, 470), 0xf, tmp);

		if (tsel_adder > rddata_en_ie_dly)
			extra_adder = tsel_adder - rddata_en_ie_dly;
		else
			extra_adder = 0;
		if (rddata_en_ie_dly > (cas_lat - 1 - hs_offset))
			tmp = tsel_adder;
		else
			tmp = rddata_en_ie_dly - 0 + extra_adder;
		/* PHY_LP4_BOOT_RDDATA_EN_DLY */
		/* DENALI_PHY_9/137/265/393 4bit offset_8 */
		mmio_clrsetbits_32(PHY_REG(i, 9), 0xf << 8, tmp << 8);
		mmio_clrsetbits_32(PHY_REG(i, 137), 0xf << 8, tmp << 8);
		mmio_clrsetbits_32(PHY_REG(i, 265), 0xf << 8, tmp << 8);
		mmio_clrsetbits_32(PHY_REG(i, 393), 0xf << 8, tmp << 8);
		/* PHY_RDDATA_EN_DLY */
		/* DENALI_PHY_85/213/341/469 4bit offset_24 */
		mmio_clrsetbits_32(PHY_REG(i, 85), 0xf << 24, tmp << 24);
		mmio_clrsetbits_32(PHY_REG(i, 213), 0xf << 24, tmp << 24);
		mmio_clrsetbits_32(PHY_REG(i, 341), 0xf << 24, tmp << 24);
		mmio_clrsetbits_32(PHY_REG(i, 469), 0xf << 24, tmp << 24);

		if (pdram_timing->mhz <= ENPER_CS_TRAINING_FREQ) {
			/*
			 * Note:Per-CS Training is not compatible at speeds
			 * under 533 MHz. If the PHY is running at a speed
			 * less than 533MHz, all phy_per_cs_training_en_X
			 * parameters must be cleared to 0.
			 */

			/*DENALI_PHY_84/212/340/468 1bit offset_16 */
			mmio_clrbits_32(PHY_REG(i, 84), 0x1 << 16);
			mmio_clrbits_32(PHY_REG(i, 212), 0x1 << 16);
			mmio_clrbits_32(PHY_REG(i, 340), 0x1 << 16);
			mmio_clrbits_32(PHY_REG(i, 468), 0x1 << 16);
		} else {
			mmio_setbits_32(PHY_REG(i, 84), 0x1 << 16);
			mmio_setbits_32(PHY_REG(i, 212), 0x1 << 16);
			mmio_setbits_32(PHY_REG(i, 340), 0x1 << 16);
			mmio_setbits_32(PHY_REG(i, 468), 0x1 << 16);
		}
		gen_rk3399_phy_dll_bypass(pdram_timing->mhz, i, fn,
					  timing_config->dram_type);
	}
}

static int to_get_clk_index(unsigned int mhz)
{
	int pll_cnt, i;

	pll_cnt = ARRAY_SIZE(dpll_rates_table);

	/* Assumming rate_table is in descending order */
	for (i = 0; i < pll_cnt; i++) {
		if (mhz >= dpll_rates_table[i].mhz)
			break;
	}

	/* if mhz lower than lowest frequency in table, use lowest frequency */
	if (i == pll_cnt)
		i = pll_cnt - 1;

	return i;
}

uint32_t ddr_get_rate(void)
{
	uint32_t refdiv, postdiv1, fbdiv, postdiv2;

	refdiv = mmio_read_32(CRU_BASE + CRU_PLL_CON(DPLL_ID, 1)) & 0x3f;
	fbdiv = mmio_read_32(CRU_BASE + CRU_PLL_CON(DPLL_ID, 0)) & 0xfff;
	postdiv1 =
		(mmio_read_32(CRU_BASE + CRU_PLL_CON(DPLL_ID, 1)) >> 8) & 0x7;
	postdiv2 =
		(mmio_read_32(CRU_BASE + CRU_PLL_CON(DPLL_ID, 1)) >> 12) & 0x7;

	return (24 / refdiv * fbdiv / postdiv1 / postdiv2) * 1000 * 1000;
}

/*
 * return: bit12: channel 1, external self-refresh
 *         bit11: channel 1, stdby_mode
 *         bit10: channel 1, self-refresh with controller and memory clock gate
 *         bit9: channel 1, self-refresh
 *         bit8: channel 1, power-down
 *
 *         bit4: channel 1, external self-refresh
 *         bit3: channel 0, stdby_mode
 *         bit2: channel 0, self-refresh with controller and memory clock gate
 *         bit1: channel 0, self-refresh
 *         bit0: channel 0, power-down
 */
uint32_t exit_low_power(void)
{
	uint32_t low_power = 0;
	uint32_t channel_mask;
	uint32_t tmp, i;

	channel_mask = (mmio_read_32(PMUGRF_BASE + PMUGRF_OSREG(2)) >> 28) &
			0x3;
	for (i = 0; i < 2; i++) {
		if (!(channel_mask & (1 << i)))
			continue;

		/* exit stdby mode */
		mmio_write_32(CIC_BASE + CIC_CTRL1,
			      (1 << (i + 16)) | (0 << i));
		/* exit external self-refresh */
		tmp = i ? 12 : 8;
		low_power |= ((mmio_read_32(PMU_BASE + PMU_SFT_CON) >> tmp) &
			      0x1) << (4 + 8 * i);
		mmio_clrbits_32(PMU_BASE + PMU_SFT_CON, 1 << tmp);
		while (!(mmio_read_32(PMU_BASE + PMU_DDR_SREF_ST) & (1 << i)))
			;
		/* exit auto low-power */
		mmio_clrbits_32(CTL_REG(i, 101), 0x7);
		/* lp_cmd to exit */
		if (((mmio_read_32(CTL_REG(i, 100)) >> 24) & 0x7f) !=
		    0x40) {
			while (mmio_read_32(CTL_REG(i, 200)) & 0x1)
				;
			mmio_clrsetbits_32(CTL_REG(i, 93), 0xffu << 24,
					   0x69 << 24);
			while (((mmio_read_32(CTL_REG(i, 100)) >> 24) & 0x7f) !=
			       0x40)
				;
		}
	}
	return low_power;
}

void resume_low_power(uint32_t low_power)
{
	uint32_t channel_mask;
	uint32_t tmp, i, val;

	channel_mask = (mmio_read_32(PMUGRF_BASE + PMUGRF_OSREG(2)) >> 28) &
		       0x3;
	for (i = 0; i < 2; i++) {
		if (!(channel_mask & (1 << i)))
			continue;

		/* resume external self-refresh */
		tmp = i ? 12 : 8;
		val = (low_power >> (4 + 8 * i)) & 0x1;
		mmio_setbits_32(PMU_BASE + PMU_SFT_CON, val << tmp);
		/* resume auto low-power */
		val = (low_power >> (8 * i)) & 0x7;
		mmio_setbits_32(CTL_REG(i, 101), val);
		/* resume stdby mode */
		val = (low_power >> (3 + 8 * i)) & 0x1;
		mmio_write_32(CIC_BASE + CIC_CTRL1,
			      (1 << (i + 16)) | (val << i));
	}
}

static void dram_low_power_config(void)
{
	uint32_t tmp, i;
	uint32_t ch_cnt = rk3399_dram_status.timing_config.ch_cnt;
	uint32_t dram_type = rk3399_dram_status.timing_config.dram_type;

	if (dram_type == DDR3)
		tmp = (2 << 16) | (0x7 << 8);
	else
		tmp = (3 << 16) | (0x7 << 8);

	for (i = 0; i < ch_cnt; i++)
		mmio_clrsetbits_32(CTL_REG(i, 101), 0x70f0f, tmp);

	/* standby idle */
	mmio_write_32(CIC_BASE + CIC_CG_WAIT_TH, 0x640008);

	if (ch_cnt == 2) {
		mmio_write_32(GRF_BASE + GRF_DDRC1_CON1,
			      (((0x1<<4) | (0x1<<5) | (0x1<<6) |
				(0x1<<7)) << 16) |
			      ((0x1<<4) | (0x0<<5) | (0x1<<6) | (0x1<<7)));
		mmio_write_32(CIC_BASE + CIC_CTRL1, 0x002a0028);
	}

	mmio_write_32(GRF_BASE + GRF_DDRC0_CON1,
		      (((0x1<<4) | (0x1<<5) | (0x1<<6) | (0x1<<7)) << 16) |
		      ((0x1<<4) | (0x0<<5) | (0x1<<6) | (0x1<<7)));
	mmio_write_32(CIC_BASE + CIC_CTRL1, 0x00150014);
}

void dram_dfs_init(void)
{
	uint32_t trefi0, trefi1, boot_freq;
	uint32_t rddqs_adjust, rddqs_slave;

	/* get sdram config for os reg */
	get_dram_drv_odt_val(sdram_config.dramtype,
			     &rk3399_dram_status.drv_odt_lp_cfg);
	sdram_timing_cfg_init(&rk3399_dram_status.timing_config,
			      &sdram_config,
			      &rk3399_dram_status.drv_odt_lp_cfg);

	trefi0 = ((mmio_read_32(CTL_REG(0, 48)) >> 16) & 0xffff) + 8;
	trefi1 = ((mmio_read_32(CTL_REG(0, 49)) >> 16) & 0xffff) + 8;

	rk3399_dram_status.index_freq[0] = trefi0 * 10 / 39;
	rk3399_dram_status.index_freq[1] = trefi1 * 10 / 39;
	rk3399_dram_status.current_index =
		(mmio_read_32(CTL_REG(0, 111)) >> 16) & 0x3;
	if (rk3399_dram_status.timing_config.dram_type == DDR3) {
		rk3399_dram_status.index_freq[0] /= 2;
		rk3399_dram_status.index_freq[1] /= 2;
	}
	boot_freq =
		rk3399_dram_status.index_freq[rk3399_dram_status.current_index];
	boot_freq = dpll_rates_table[to_get_clk_index(boot_freq)].mhz;
	rk3399_dram_status.boot_freq = boot_freq;
	rk3399_dram_status.index_freq[rk3399_dram_status.current_index] =
		boot_freq;
	rk3399_dram_status.index_freq[(rk3399_dram_status.current_index + 1) &
				      0x1] = 0;
	rk3399_dram_status.low_power_stat = 0;
	/*
	 * following register decide if NOC stall the access request
	 * or return error when NOC being idled. when doing ddr frequency
	 * scaling in M0 or DCF, we need to make sure noc stall the access
	 * request, if return error cpu may data abort when ddr frequency
	 * changing. it don't need to set this register every times,
	 * so we init this register in function dram_dfs_init().
	 */
	mmio_write_32(GRF_BASE + GRF_SOC_CON(0), 0xffffffff);
	mmio_write_32(GRF_BASE + GRF_SOC_CON(1), 0xffffffff);
	mmio_write_32(GRF_BASE + GRF_SOC_CON(2), 0xffffffff);
	mmio_write_32(GRF_BASE + GRF_SOC_CON(3), 0xffffffff);
	mmio_write_32(GRF_BASE + GRF_SOC_CON(4), 0x70007000);

	/* Disable multicast */
	mmio_clrbits_32(PHY_REG(0, 896), 1);
	mmio_clrbits_32(PHY_REG(1, 896), 1);
	dram_low_power_config();

	/*
	 * If boot_freq isn't in the bypass mode, it can get the
	 * rddqs_delay_ps from the result of gate training
	 */
	if (((mmio_read_32(PHY_REG(0, 86)) >> 8) & 0xf) != 0xc) {

		/*
		 * Select PHY's frequency set to current_index
		 * index for get the result of gate Training
		 * from registers
		 */
		mmio_clrsetbits_32(PHY_REG(0, 896), 0x3 << 8,
				   rk3399_dram_status.current_index << 8);
		rddqs_slave = (mmio_read_32(PHY_REG(0, 77)) >> 16) & 0x3ff;
		rddqs_slave = rddqs_slave * 1000000 / boot_freq / 512;

		rddqs_adjust = mmio_read_32(PHY_REG(0, 78)) & 0xf;
		rddqs_adjust = rddqs_adjust * 1000000 / boot_freq;
		rddqs_delay_ps = rddqs_slave + rddqs_adjust -
				(1000000 / boot_freq / 2);
	} else {
		rddqs_delay_ps = 3500;
	}
}

/*
 * arg0: bit0-7: sr_idle; bit8-15:sr_mc_gate_idle; bit16-31: standby idle
 * arg1: bit0-11: pd_idle; bit 16-27: srpd_lite_idle
 * arg2: bit0: if odt en
 */
uint32_t dram_set_odt_pd(uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
	struct drv_odt_lp_config *lp_cfg = &rk3399_dram_status.drv_odt_lp_cfg;
	uint32_t *low_power = &rk3399_dram_status.low_power_stat;
	uint32_t dram_type, ch_count, pd_tmp, sr_tmp, i;

	dram_type = rk3399_dram_status.timing_config.dram_type;
	ch_count = rk3399_dram_status.timing_config.ch_cnt;

	lp_cfg->sr_idle = arg0 & 0xff;
	lp_cfg->sr_mc_gate_idle = (arg0 >> 8) & 0xff;
	lp_cfg->standby_idle = (arg0 >> 16) & 0xffff;
	lp_cfg->pd_idle = arg1 & 0xfff;
	lp_cfg->srpd_lite_idle = (arg1 >> 16) & 0xfff;

	rk3399_dram_status.timing_config.odt = arg2 & 0x1;

	exit_low_power();

	*low_power = 0;

	/* pd_idle en */
	if (lp_cfg->pd_idle)
		*low_power |= ((1 << 0) | (1 << 8));
	/* sr_idle en srpd_lite_idle */
	if (lp_cfg->sr_idle | lp_cfg->srpd_lite_idle)
		*low_power |= ((1 << 1) | (1 << 9));
	/* sr_mc_gate_idle */
	if (lp_cfg->sr_mc_gate_idle)
		*low_power |= ((1 << 2) | (1 << 10));
	/* standbyidle */
	if (lp_cfg->standby_idle) {
		if (rk3399_dram_status.timing_config.ch_cnt == 2)
			*low_power |= ((1 << 3) | (1 << 11));
		else
			*low_power |= (1 << 3);
	}

	pd_tmp = arg1;
	if (dram_type != LPDDR4)
		pd_tmp = arg1 & 0xfff;
	sr_tmp = arg0 & 0xffff;
	for (i = 0; i < ch_count; i++) {
		mmio_write_32(CTL_REG(i, 102), pd_tmp);
		mmio_clrsetbits_32(CTL_REG(i, 103), 0xffff, sr_tmp);
	}
	mmio_write_32(CIC_BASE + CIC_IDLE_TH, (arg0 >> 16) & 0xffff);

	return 0;
}

static void m0_configure_ddr(struct pll_div pll_div, uint32_t ddr_index)
{
	mmio_write_32(M0_PARAM_ADDR + PARAM_DPLL_CON0, FBDIV(pll_div.fbdiv));
	mmio_write_32(M0_PARAM_ADDR + PARAM_DPLL_CON1,
		      POSTDIV2(pll_div.postdiv2) | POSTDIV1(pll_div.postdiv1) |
		      REFDIV(pll_div.refdiv));

	mmio_write_32(M0_PARAM_ADDR + PARAM_DRAM_FREQ, pll_div.mhz);

	mmio_write_32(M0_PARAM_ADDR + PARAM_FREQ_SELECT, ddr_index << 4);
	dmbst();
	m0_configure_execute_addr(M0_BINCODE_BASE);
}

static uint32_t prepare_ddr_timing(uint32_t mhz)
{
	uint32_t index;
	struct dram_timing_t dram_timing;

	rk3399_dram_status.timing_config.freq = mhz;

	if (mhz < 300)
		rk3399_dram_status.timing_config.dllbp = 1;
	else
		rk3399_dram_status.timing_config.dllbp = 0;

	if (rk3399_dram_status.timing_config.odt == 1)
		gen_rk3399_set_odt(1);

	index = (rk3399_dram_status.current_index + 1) & 0x1;

	/*
	 * checking if having available gate traiing timing for
	 * target freq.
	 */
	dram_get_parameter(&rk3399_dram_status.timing_config, &dram_timing);
	gen_rk3399_ctl_params(&rk3399_dram_status.timing_config,
			      &dram_timing, index);
	gen_rk3399_pi_params(&rk3399_dram_status.timing_config,
			     &dram_timing, index);
	gen_rk3399_phy_params(&rk3399_dram_status.timing_config,
			      &rk3399_dram_status.drv_odt_lp_cfg,
			      &dram_timing, index);
	rk3399_dram_status.index_freq[index] = mhz;

	return index;
}

uint32_t ddr_set_rate(uint32_t hz)
{
	uint32_t low_power, index, ddr_index;
	uint32_t mhz = hz / (1000 * 1000);

	if (mhz ==
	    rk3399_dram_status.index_freq[rk3399_dram_status.current_index])
		return mhz;

	index = to_get_clk_index(mhz);
	mhz = dpll_rates_table[index].mhz;

	ddr_index = prepare_ddr_timing(mhz);
	gen_rk3399_enable_training(rk3399_dram_status.timing_config.ch_cnt,
				   mhz);
	if (ddr_index > 1)
		goto out;

	/*
	 * Make sure the clock is enabled. The M0 clocks should be on all of the
	 * time during S0.
	 */
	m0_configure_ddr(dpll_rates_table[index], ddr_index);
	m0_start();
	m0_wait_done();
	m0_stop();

	if (rk3399_dram_status.timing_config.odt == 0)
		gen_rk3399_set_odt(0);

	rk3399_dram_status.current_index = ddr_index;
	low_power = rk3399_dram_status.low_power_stat;
	resume_low_power(low_power);
out:
	gen_rk3399_disable_training(rk3399_dram_status.timing_config.ch_cnt);
	return mhz;
}

uint32_t ddr_round_rate(uint32_t hz)
{
	int index;
	uint32_t mhz = hz / (1000 * 1000);

	index = to_get_clk_index(mhz);

	return dpll_rates_table[index].mhz * 1000 * 1000;
}

void ddr_prepare_for_sys_suspend(void)
{
	uint32_t mhz =
		rk3399_dram_status.index_freq[rk3399_dram_status.current_index];

	/*
	 * If we're not currently at the boot (assumed highest) frequency, we
	 * need to change frequencies to configure out current index.
	 */
	rk3399_suspend_status.freq = mhz;
	exit_low_power();
	rk3399_suspend_status.low_power_stat =
		rk3399_dram_status.low_power_stat;
	rk3399_suspend_status.odt = rk3399_dram_status.timing_config.odt;
	rk3399_dram_status.low_power_stat = 0;
	rk3399_dram_status.timing_config.odt = 1;
	if (mhz != rk3399_dram_status.boot_freq)
		ddr_set_rate(rk3399_dram_status.boot_freq * 1000 * 1000);

	/*
	 * This will configure the other index to be the same frequency as the
	 * current one. We retrain both indices on resume, so both have to be
	 * setup for the same frequency.
	 */
	prepare_ddr_timing(rk3399_dram_status.boot_freq);
}

void ddr_prepare_for_sys_resume(void)
{
	/* Disable multicast */
	mmio_clrbits_32(PHY_REG(0, 896), 1);
	mmio_clrbits_32(PHY_REG(1, 896), 1);

	/* The suspend code changes the current index, so reset it now. */
	rk3399_dram_status.current_index =
		(mmio_read_32(CTL_REG(0, 111)) >> 16) & 0x3;
	rk3399_dram_status.low_power_stat =
		rk3399_suspend_status.low_power_stat;
	rk3399_dram_status.timing_config.odt = rk3399_suspend_status.odt;

	/*
	 * Set the saved frequency from suspend if it's different than the
	 * current frequency.
	 */
	if (rk3399_suspend_status.freq !=
	    rk3399_dram_status.index_freq[rk3399_dram_status.current_index]) {
		ddr_set_rate(rk3399_suspend_status.freq * 1000 * 1000);
		return;
	}

	gen_rk3399_set_odt(rk3399_dram_status.timing_config.odt);
	resume_low_power(rk3399_dram_status.low_power_stat);
}
