/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <debug.h>
#include <mmio.h>
#include <plat_private.h>
#include "dram.h"
#include "dram_spec_timing.h"
#include "string.h"
#include "soc.h"
#include "pmu.h"

#include <delay_timer.h>

#define CTL_TRAINING	(1)
#define PI_TRAINING		(!CTL_TRAINING)

#define EN_READ_GATE_TRAINING	(1)
#define EN_CA_TRAINING		(0)
#define EN_WRITE_LEVELING	(0)
#define EN_READ_LEVELING	(0)
#define EN_WDQ_LEVELING	(0)

#define ENPER_CS_TRAINING_FREQ	(933)

struct pll_div {
	unsigned int mhz;
	unsigned int refdiv;
	unsigned int fbdiv;
	unsigned int postdiv1;
	unsigned int postdiv2;
	unsigned int frac;
	unsigned int freq;
};

static const struct pll_div dpll_rates_table[] = {

	/* _mhz, _refdiv, _fbdiv, _postdiv1, _postdiv2 */
	{.mhz = 933, .refdiv = 3, .fbdiv = 350, .postdiv1 = 3, .postdiv2 = 1},
	{.mhz = 800, .refdiv = 1, .fbdiv = 100, .postdiv1 = 3, .postdiv2 = 1},
	{.mhz = 732, .refdiv = 1, .fbdiv = 61, .postdiv1 = 2, .postdiv2 = 1},
	{.mhz = 666, .refdiv = 1, .fbdiv = 111, .postdiv1 = 4, .postdiv2 = 1},
	{.mhz = 600, .refdiv = 1, .fbdiv = 50, .postdiv1 = 2, .postdiv2 = 1},
	{.mhz = 528, .refdiv = 1, .fbdiv = 66, .postdiv1 = 3, .postdiv2 = 1},
	{.mhz = 400, .refdiv = 1, .fbdiv = 50, .postdiv1 = 3, .postdiv2 = 1},
	{.mhz = 300, .refdiv = 1, .fbdiv = 50, .postdiv1 = 4, .postdiv2 = 1},
	{.mhz = 200, .refdiv = 1, .fbdiv = 50, .postdiv1 = 3, .postdiv2 = 2},
};

static struct rk3399_ddr_cic_regs *const rk3399_ddr_cic = (void *)CIC_BASE;
static struct rk3399_ddr_pctl_regs *const rk3399_ddr_pctl[2] = {
	(void *)DDRC0_BASE, (void *)DDRC1_BASE
};

static struct rk3399_ddr_pi_regs *const rk3399_ddr_pi[2] = {
	(void *)DDRC0_PI_BASE, (void *)DDRC1_PI_BASE
};

static struct rk3399_ddr_publ_regs *const rk3399_ddr_publ[2] = {
	(void *)DDRC0_PHY_BASE, (void *)DDRC1_PHY_BASE
};

struct rk3399_dram_status {
	uint32_t current_index;
	uint32_t index_freq[2];
	uint32_t low_power_stat;
	struct timing_related_config timing_config;
	struct drv_odt_lp_config drv_odt_lp_cfg;
};

static struct rk3399_dram_status rk3399_dram_status;
static struct ddr_dts_config_timing dts_parameter = {
	.available = 0
};

static struct rk3399_sdram_default_config ddr3_default_config = {
	.bl = 8,
	.ap = 0,
	.dramds = 40,
	.dramodt = 120,
	.burst_ref_cnt = 1,
	.zqcsi = 0
};

static struct drv_odt_lp_config ddr3_drv_odt_default_config = {
	.ddr3_speed_bin = DDR3_DEFAULT,
	.pd_idle = 0,
	.sr_idle = 0,
	.sr_mc_gate_idle = 0,
	.srpd_lite_idle = 0,
	.standby_idle = 0,

	.ddr3_dll_dis_freq = 300,
	.phy_dll_dis_freq = 125,
	.odt_dis_freq = 933,

	.dram_side_drv = 40,
	.dram_side_dq_odt = 120,
	.dram_side_ca_odt = 120,

	.phy_side_ca_drv = 40,
	.phy_side_ck_cs_drv = 40,
	.phy_side_dq_drv = 40,
	.phy_side_odt = 240,
};

static struct rk3399_sdram_default_config lpddr3_default_config = {
	.bl = 8,
	.ap = 0,
	.dramds = 34,
	.dramodt = 240,
	.burst_ref_cnt = 1,
	.zqcsi = 0
};

static struct drv_odt_lp_config lpddr3_drv_odt_default_config = {
	.ddr3_speed_bin = DDR3_DEFAULT,
	.pd_idle = 0,
	.sr_idle = 0,
	.sr_mc_gate_idle = 0,
	.srpd_lite_idle = 0,
	.standby_idle = 0,

	.ddr3_dll_dis_freq = 300,
	.phy_dll_dis_freq = 125,
	.odt_dis_freq = 666,

	.dram_side_drv = 40,
	.dram_side_dq_odt = 120,
	.dram_side_ca_odt = 120,

	.phy_side_ca_drv = 40,
	.phy_side_ck_cs_drv = 40,
	.phy_side_dq_drv = 40,
	.phy_side_odt = 240,
};

static struct rk3399_sdram_default_config lpddr4_default_config = {
	.bl = 16,
	.ap = 0,
	.dramds = 40,
	.dramodt = 240,
	.caodt = 240,
	.burst_ref_cnt = 1,
	.zqcsi = 0
};

static struct drv_odt_lp_config lpddr4_drv_odt_default_config = {
	.ddr3_speed_bin = DDR3_DEFAULT,
	.pd_idle = 0,
	.sr_idle = 0,
	.sr_mc_gate_idle = 0,
	.srpd_lite_idle = 0,
	.standby_idle = 0,

	.ddr3_dll_dis_freq = 300,
	.phy_dll_dis_freq = 125,
	.odt_dis_freq = 933,

	.dram_side_drv = 60,
	.dram_side_dq_odt = 40,
	.dram_side_ca_odt = 40,

	.phy_side_ca_drv = 40,
	.phy_side_ck_cs_drv = 80,
	.phy_side_dq_drv = 80,
	.phy_side_odt = 60,
};

uint32_t dcf_code[] = {
#include "dcf_code.inc"
};


#define write_32(addr, value)\
	mmio_write_32((uintptr_t)(addr), (uint32_t)(value))

#define read_32(addr) \
		mmio_read_32((uintptr_t)(addr))
#define clrbits_32(addr, clear)\
		mmio_clrbits_32((uintptr_t)(addr), (uint32_t)(clear))
#define setbits_32(addr, set)\
	mmio_setbits_32((uintptr_t)(addr), (uint32_t)(set))
#define clrsetbits_32(addr, clear, set)\
	mmio_clrsetbits_32((uintptr_t)(addr), (uint32_t)(clear),\
					(uint32_t)(set))

#define DCF_START_ADDR	(SRAM_BASE + 0x1400)
#define DCF_PARAM_ADDR	(SRAM_BASE + 0x1000)

/* DCF_PAMET */
#define PARAM_DRAM_FREQ		(0)
#define PARAM_DPLL_CON0		(4)
#define PARAM_DPLL_CON1		(8)
#define PARAM_DPLL_CON2		(0xc)
#define PARAM_DPLL_CON3		(0x10)
#define PARAM_DPLL_CON4		(0x14)
#define PARAM_DPLL_CON5		(0x18)
/* equal to fn<<4 */
#define PARAM_FREQ_SELECT	(0x1c)

static unsigned int get_cs_die_capability(struct rk3399_sdram_config
					  *psdram_config, unsigned int channel,
					  unsigned int cs)
{
	unsigned int die;
	unsigned int cs_cap;
	unsigned int row[2];

	row[0] = psdram_config->ch[channel].cs0_row;
	row[1] = psdram_config->ch[channel].cs1_row;
	die = psdram_config->ch[channel].bus_width /
	    psdram_config->ch[channel].each_die_bus_width;
	cs_cap = (1 << (row[cs] +
			(psdram_config->ch[channel].bank / 4 + 1) +
			psdram_config->ch[channel].col +
			(psdram_config->ch[channel].bus_width / 16)));
	if (psdram_config->ch[channel].each_die_6gb_or_12gb)
		cs_cap = cs_cap * 3 / 4;

	return (cs_cap / die);
}

static void sdram_config_init(struct rk3399_sdram_config *psdram_config)
{
	uint32_t os_reg2_val, i;

	os_reg2_val = read_32(PMUGRF_BASE + PMUGRF_OSREG(2));

	for (i = 0; i < READ_CH_CNT(os_reg2_val); i++) {
		psdram_config->ch[i].bank = 1 << READ_BK_INFO(os_reg2_val, i);
		psdram_config->ch[i].bus_width =
		    8 * (1 << READ_BW_INFO(os_reg2_val, i));
		psdram_config->ch[i].col = READ_COL_INFO(os_reg2_val, i);
		psdram_config->ch[i].cs0_row =
		    READ_CS0_ROW_INFO(os_reg2_val, i);
		psdram_config->ch[i].cs1_row =
		    READ_CS1_ROW_INFO(os_reg2_val, i);
		psdram_config->ch[i].cs_cnt = READ_CS_INFO(os_reg2_val, i);
		psdram_config->ch[i].each_die_6gb_or_12gb =
		    READ_CH_ROW_INFO(os_reg2_val, i);
		psdram_config->ch[i].each_die_bus_width =
		    8 * (1 << READ_DIE_BW_INFO(os_reg2_val, i));
	}
	psdram_config->dramtype = READ_DRAMTYPE_INFO(os_reg2_val);
	psdram_config->channal_num = READ_CH_CNT(os_reg2_val);
}

static void drv_odt_lp_cfg_init(uint32_t dram_type,
				struct ddr_dts_config_timing *dts_timing,
				struct drv_odt_lp_config *drv_config)
{
	if ((dts_timing) && (dts_timing->available)) {
		drv_config->ddr3_speed_bin = dts_timing->ddr3_speed_bin;
		drv_config->pd_idle = dts_timing->pd_idle;
		drv_config->sr_idle = dts_timing->sr_idle;
		drv_config->sr_mc_gate_idle = dts_timing->sr_mc_gate_idle;
		drv_config->srpd_lite_idle = dts_timing->srpd_lite_idle;
		drv_config->standby_idle = dts_timing->standby_idle;
		drv_config->ddr3_dll_dis_freq = dts_timing->ddr3_dll_dis_freq;
		drv_config->phy_dll_dis_freq = dts_timing->phy_dll_dis_freq;
	}

	switch (dram_type) {
	case DDR3:
		if ((dts_timing) && (dts_timing->available)) {
			drv_config->odt_dis_freq =
			    dts_timing->ddr3_odt_dis_freq;
			drv_config->dram_side_drv = dts_timing->ddr3_drv;
			drv_config->dram_side_dq_odt = dts_timing->ddr3_odt;
			drv_config->phy_side_ca_drv =
			    dts_timing->phy_ddr3_ca_drv;
			drv_config->phy_side_ck_cs_drv =
			    dts_timing->phy_ddr3_ca_drv;
			drv_config->phy_side_dq_drv =
			    dts_timing->phy_ddr3_dq_drv;
			drv_config->phy_side_odt = dts_timing->phy_ddr3_odt;
		} else {
			memcpy(drv_config, &ddr3_drv_odt_default_config,
			       sizeof(struct drv_odt_lp_config));
		}
		break;
	case LPDDR3:
		if ((dts_timing) && (dts_timing->available)) {
			drv_config->odt_dis_freq =
			    dts_timing->lpddr3_odt_dis_freq;
			drv_config->dram_side_drv = dts_timing->lpddr3_drv;
			drv_config->dram_side_dq_odt = dts_timing->lpddr3_odt;
			drv_config->phy_side_ca_drv =
			    dts_timing->phy_lpddr3_ca_drv;
			drv_config->phy_side_ck_cs_drv =
			    dts_timing->phy_lpddr3_ca_drv;
			drv_config->phy_side_dq_drv =
			    dts_timing->phy_lpddr3_dq_drv;
			drv_config->phy_side_odt = dts_timing->phy_lpddr3_odt;

		} else {
			memcpy(drv_config, &lpddr3_drv_odt_default_config,
			       sizeof(struct drv_odt_lp_config));
		}
		break;
	case LPDDR4:
	default:
		if ((dts_timing) && (dts_timing->available)) {
			drv_config->odt_dis_freq =
			    dts_timing->lpddr4_odt_dis_freq;
			drv_config->dram_side_drv = dts_timing->lpddr4_drv;
			drv_config->dram_side_dq_odt =
			    dts_timing->lpddr4_dq_odt;
			drv_config->dram_side_ca_odt =
			    dts_timing->lpddr4_ca_odt;
			drv_config->phy_side_ca_drv =
			    dts_timing->phy_lpddr4_ca_drv;
			drv_config->phy_side_ck_cs_drv =
			    dts_timing->phy_lpddr4_ck_cs_drv;
			drv_config->phy_side_dq_drv =
			    dts_timing->phy_lpddr4_dq_drv;
			drv_config->phy_side_odt = dts_timing->phy_lpddr4_odt;
		} else {
			memcpy(drv_config, &lpddr4_drv_odt_default_config,
			       sizeof(struct drv_odt_lp_config));
		}
		break;
	}

	switch (drv_config->phy_side_ca_drv) {
	case 240:
		drv_config->phy_side_ca_drv = PHY_DRV_ODT_240;
		break;
	case 120:
		drv_config->phy_side_ca_drv = PHY_DRV_ODT_120;
		break;
	case 80:
		drv_config->phy_side_ca_drv = PHY_DRV_ODT_80;
		break;
	case 60:
		drv_config->phy_side_ca_drv = PHY_DRV_ODT_60;
		break;
	case 48:
		drv_config->phy_side_ca_drv = PHY_DRV_ODT_48;
		break;
	case 40:
		drv_config->phy_side_ca_drv = PHY_DRV_ODT_40;
		break;
	default:
		drv_config->phy_side_ca_drv = PHY_DRV_ODT_34_3;
		break;
	};

	switch (drv_config->phy_side_ck_cs_drv) {
	case 240:
		drv_config->phy_side_ck_cs_drv = PHY_DRV_ODT_240;
		break;
	case 120:
		drv_config->phy_side_ck_cs_drv = PHY_DRV_ODT_120;
		break;
	case 80:
		drv_config->phy_side_ck_cs_drv = PHY_DRV_ODT_80;
		break;
	case 60:
		drv_config->phy_side_ck_cs_drv = PHY_DRV_ODT_60;
		break;
	case 48:
		drv_config->phy_side_ck_cs_drv = PHY_DRV_ODT_48;
		break;
	case 40:
		drv_config->phy_side_ck_cs_drv = PHY_DRV_ODT_40;
		break;
	default:
		drv_config->phy_side_ck_cs_drv = PHY_DRV_ODT_34_3;
		break;
	}

	switch (drv_config->phy_side_dq_drv) {
	case 240:
		drv_config->phy_side_dq_drv = PHY_DRV_ODT_240;
		break;
	case 120:
		drv_config->phy_side_dq_drv = PHY_DRV_ODT_120;
		break;
	case 80:
		drv_config->phy_side_dq_drv = PHY_DRV_ODT_80;
		break;
	case 60:
		drv_config->phy_side_dq_drv = PHY_DRV_ODT_60;
		break;
	case 48:
		drv_config->phy_side_dq_drv = PHY_DRV_ODT_48;
		break;
	case 40:
		drv_config->phy_side_dq_drv = PHY_DRV_ODT_40;
		break;
	default:
		drv_config->phy_side_dq_drv = PHY_DRV_ODT_34_3;
		break;
	}

	switch (drv_config->phy_side_odt) {
	case 240:
		drv_config->phy_side_odt = PHY_DRV_ODT_240;
		break;
	case 120:
		drv_config->phy_side_odt = PHY_DRV_ODT_120;
		break;
	case 80:
		drv_config->phy_side_odt = PHY_DRV_ODT_80;
		break;
	case 60:
		drv_config->phy_side_odt = PHY_DRV_ODT_60;
		break;
	case 48:
		drv_config->phy_side_odt = PHY_DRV_ODT_48;
		break;
	case 40:
		drv_config->phy_side_odt = PHY_DRV_ODT_40;
		break;
	default:
		drv_config->phy_side_odt = PHY_DRV_ODT_34_3;
		break;
	}
}

static void sdram_timing_cfg_init(struct timing_related_config *ptiming_config,
				  struct rk3399_sdram_config *psdram_config,
				  struct drv_odt_lp_config *drv_config)
{
	uint32_t i, j;

	for (i = 0; i < psdram_config->channal_num; i++) {
		ptiming_config->dram_info[i].speed_rate =
		    drv_config->ddr3_speed_bin;
		ptiming_config->dram_info[i].cs_cnt =
		    psdram_config->ch[i].cs_cnt;
		for (j = 0; j < psdram_config->ch[i].cs_cnt; j++) {
			ptiming_config->dram_info[i].per_die_capability[j] =
			    get_cs_die_capability(psdram_config, i, j);
		}
	}
	ptiming_config->dram_type = psdram_config->dramtype;
	ptiming_config->ch_cnt = psdram_config->channal_num;
	switch (psdram_config->dramtype) {
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
	}
	ptiming_config->dramds = drv_config->dram_side_drv;
	ptiming_config->dramodt = drv_config->dram_side_dq_odt;
	ptiming_config->caodt = drv_config->dram_side_ca_odt;
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
		printf("get_pi_tdfi_phy_rdlat:dramtype unsupport\n");
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
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[5], tmp);

			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[22],
				      0xffff, pdram_timing->tdllk);

			write_32(&rk3399_ddr_pctl[i]->denali_ctl[32],
				 (pdram_timing->tmod << 8) |
				 pdram_timing->tmrd);

			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[59],
				      0xffff << 16,
				      (pdram_timing->txsr -
				       pdram_timing->trcd) << 16);
		} else if (timing_config->dram_type == LPDDR4) {
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[5],
				 pdram_timing->tinit1 + pdram_timing->tinit3);
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[32],
				 (pdram_timing->tmrd << 8) |
				 pdram_timing->tmrd);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[59],
				      0xffff << 16, pdram_timing->txsr << 16);
		} else {
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[5],
				 pdram_timing->tinit1);
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[7],
				 pdram_timing->tinit4);
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[32],
				 (pdram_timing->tmrd << 8) |
				 pdram_timing->tmrd);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[59],
				      0xffff << 16, pdram_timing->txsr << 16);
		}
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[6],
			 pdram_timing->tinit3);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[8],
			 pdram_timing->tinit5);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[23], (0x7f << 16),
			      ((pdram_timing->cl * 2) << 16));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[23], (0x1f << 24),
			      (pdram_timing->cwl << 24));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[24], 0x3f,
			      pdram_timing->al);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[26], 0xffff << 16,
			      (pdram_timing->trc << 24) |
			      (pdram_timing->trrd << 16));
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[27],
			 (pdram_timing->tfaw << 24) |
			 (pdram_timing->trppb << 16) |
			 (pdram_timing->twtr << 8) | pdram_timing->tras_min);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[31], 0xff << 24,
			      max(4, pdram_timing->trtp) << 24);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[33],
			 (pdram_timing->tcke << 24) | pdram_timing->tras_max);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[34], 0xff,
			      max(1, pdram_timing->tckesr));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[39],
			      (0x3f << 16) | (0xff << 8),
			      (pdram_timing->twr << 16) |
			      (pdram_timing->trcd << 8));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[42], 0x1f << 16,
			      pdram_timing->tmrz << 16);
		tmp = pdram_timing->tdal ? pdram_timing->tdal :
		       (pdram_timing->twr + pdram_timing->trp);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[44], 0xff, tmp);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[45], 0xff,
			      pdram_timing->trp);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[48],
			 ((pdram_timing->trefi - 8) << 16) |
			 pdram_timing->trfc);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[52], 0xffff,
			      pdram_timing->txp);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[53], 0xffff << 16,
			      pdram_timing->txpdll << 16);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[55], 0xf << 24,
			      pdram_timing->tcscke << 24);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[55], 0xff,
			      pdram_timing->tmrri);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[56],
			 (pdram_timing->tzqcke << 24) |
			 (pdram_timing->tmrwckel << 16) |
			 (pdram_timing->tckehcs << 8) | pdram_timing->tckelcs);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[60], 0xffff,
			      pdram_timing->txsnr);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[62], 0xffff << 16,
			      (pdram_timing->tckehcmd << 24) |
			      (pdram_timing->tckelcmd << 16));
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[63],
			 (pdram_timing->tckelpd << 24) |
			 (pdram_timing->tescke << 16) |
			 (pdram_timing->tsr << 8) | pdram_timing->tckckel);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[64], 0xfff,
			      (pdram_timing->tcmdcke << 8) |
			      pdram_timing->tcsckeh);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[92],
			      (0xffff << 8),
			      (pdram_timing->tcksrx << 16) |
			      (pdram_timing->tcksre << 8));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[108], (0x1 << 24),
			      (timing_config->dllbp << 24));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[122],
			      (0x3FF << 16),
			      (pdram_timing->tvrcg_enable << 16));
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[123],
			 (pdram_timing->tfc_long << 16) |
			 pdram_timing->tvrcg_disable);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[124],
			 (pdram_timing->tvref_long << 16) |
			 (pdram_timing->tckfspx << 8) |
			 pdram_timing->tckfspe);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[133],
			 (pdram_timing->mr[1] << 16) | pdram_timing->mr[0]);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[134], 0xffff,
			      pdram_timing->mr[2]);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[138], 0xffff,
			      pdram_timing->mr[3]);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[139], 0xff << 24,
			      pdram_timing->mr11 << 24);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[147],
			 (pdram_timing->mr[1] << 16) | pdram_timing->mr[0]);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[148], 0xffff,
			      pdram_timing->mr[2]);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[152], 0xffff,
			      pdram_timing->mr[3]);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[153], 0xff << 24,
			      pdram_timing->mr11 << 24);
		if (timing_config->dram_type == LPDDR4) {
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[140],
				      0xffff << 16, pdram_timing->mr12 << 16);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[142],
				      0xffff << 16, pdram_timing->mr14 << 16);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[145],
				      0xffff << 16, pdram_timing->mr22 << 16);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[154],
				      0xffff << 16, pdram_timing->mr12 << 16);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[156],
				      0xffff << 16, pdram_timing->mr14 << 16);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[159],
				      0xffff << 16, pdram_timing->mr22 << 16);
		}
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[179], 0xfff << 8,
			      pdram_timing->tzqinit << 8);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[180],
			 (pdram_timing->tzqcs << 16) |
			 (pdram_timing->tzqinit / 2));
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[181],
			 (pdram_timing->tzqlat << 16) | pdram_timing->tzqcal);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[212], 0xff << 8,
			      pdram_timing->todton << 8);

		if (timing_config->odt) {
			setbits_32(&rk3399_ddr_pctl[i]->denali_ctl[213],
				   1 << 16);
			if (timing_config->freq < 400)
				tmp = 4 << 24;
			else
				tmp = 8 << 24;
		} else {
			clrbits_32(&rk3399_ddr_pctl[i]->denali_ctl[213],
				   1 << 16);
			tmp = 2 << 24;
		}

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[216],
			      0x1f << 24, tmp);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[221],
			      (0x3 << 16) | (0xf << 8),
			      (pdram_timing->tdqsck << 16) |
			      (pdram_timing->tdqsck_max << 8));
		tmp =
		    (get_wrlat_adj(timing_config->dram_type, pdram_timing->cwl)
		     << 8) | get_rdlat_adj(timing_config->dram_type,
					   pdram_timing->cl);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[284], 0xffff,
			      tmp);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[82], 0xffff << 16,
			      (4 * pdram_timing->trefi) << 16);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[83], 0xffff,
			      (2 * pdram_timing->trefi) & 0xffff);

		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			tmp = get_pi_wrlat(pdram_timing, timing_config);
			tmp1 = get_pi_todtoff_max(pdram_timing, timing_config);
			tmp = (tmp > tmp1) ? (tmp - tmp1) : 0;
		} else {
			tmp = 0;
		}
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[214], 0x3f << 16,
			      (tmp & 0x3f) << 16);

		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			/* min_rl_preamble= cl+TDQSCK_MIN-1 */
			tmp = pdram_timing->cl +
			    get_pi_todtoff_min(pdram_timing, timing_config) - 1;
			/* todtoff_max */
			tmp1 = get_pi_todtoff_max(pdram_timing, timing_config);
			tmp = (tmp > tmp1) ? (tmp - tmp1) : 0;
		} else {
			tmp = pdram_timing->cl - pdram_timing->cwl;
		}
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[215], 0x3f << 8,
			      (tmp & 0x3f) << 8);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[275], 0xff << 16,
			      (get_pi_tdfi_phy_rdlat
			       (pdram_timing, timing_config)
			       & 0xff) << 16);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[277], 0xffff,
			      (2 * pdram_timing->trefi) & 0xffff);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[282], 0xffff,
			      (2 * pdram_timing->trefi) & 0xffff);

		write_32(&rk3399_ddr_pctl[i]->denali_ctl[283],
			 20 * pdram_timing->trefi);

		/* CTL_308 TDFI_CALVL_CAPTURE_F0:RW:16:10 */
		tmp1 = 20000 / (1000000 / pdram_timing->mhz) + 1;
		if ((20000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		tmp = (tmp1 >> 1) + (tmp1 % 2) + 5;
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[308], 0x3ff << 16,
				tmp << 16);

		/* CTL_308 TDFI_CALVL_CC_F0:RW:0:10 */
		tmp = tmp + 18;
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[308], 0x3ff,
				tmp);

		/* CTL_314 TDFI_WRCSLAT_F0:RW:8:8 */
		tmp1 = get_pi_wrlat_adj(pdram_timing, timing_config);
		if (timing_config->freq <= ENPER_CS_TRAINING_FREQ) {
			if (tmp1 < 5) {
				if (tmp1 == 0)
					tmp = 0;
				else
					tmp = tmp1 - 1;
			} else {
				tmp = tmp1 - 5;
			}
		} else {
			tmp = tmp1 - 2;
		}
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[314], 0xff << 8,
				tmp << 8);

		/* CTL_314 TDFI_RDCSLAT_F0:RW:0:8 */
		if ((timing_config->freq <= ENPER_CS_TRAINING_FREQ) &&
			(pdram_timing->cl >= 5))
			tmp = pdram_timing->cl - 5;
		else
			tmp = pdram_timing->cl - 2;
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[314], 0xff,
				tmp);
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
			    ((700000 + 10) * timing_config->freq +
			      999) / 1000;
			tmp +=
			    pdram_timing->txsnr + (pdram_timing->tmrd * 3) +
			    pdram_timing->tmod + pdram_timing->tzqinit;
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[9], tmp);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[22],
				      0xffff << 16, pdram_timing->tdllk << 16);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[34],
				      0xffffff00,
				      (pdram_timing->tmod << 24) |
				      (pdram_timing->tmrd << 16) |
				      (pdram_timing->trtp << 8));
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[60],
				      0xffff << 16,
				      (pdram_timing->txsr -
				       pdram_timing->trcd) << 16);
		} else if (timing_config->dram_type == LPDDR4) {
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[9],
				 pdram_timing->tinit1 + pdram_timing->tinit3);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[34],
				      0xffffff00,
				      (pdram_timing->tmrd << 24) |
				      (pdram_timing->tmrd << 16) |
				      (pdram_timing->trtp << 8));
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[60],
				      0xffff << 16, pdram_timing->txsr << 16);
		} else {
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[9],
				 pdram_timing->tinit1);
			write_32(&rk3399_ddr_pctl[i]->denali_ctl[11],
				 pdram_timing->tinit4);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[34],
				      0xffffff00,
				      (pdram_timing->tmrd << 24) |
				      (pdram_timing->tmrd << 16) |
				      (pdram_timing->trtp << 8));
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[60],
				      0xffff << 16, pdram_timing->txsr << 16);
		}
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[10],
			 pdram_timing->tinit3);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[12],
			 pdram_timing->tinit5);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[24], (0x7f << 8),
			      ((pdram_timing->cl * 2) << 8));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[24], (0x1f << 16),
			      (pdram_timing->cwl << 16));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[24], 0x3f << 24,
			      pdram_timing->al << 24);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[28], 0xffffff00,
			      (pdram_timing->tras_min << 24) |
			      (pdram_timing->trc << 16) |
			      (pdram_timing->trrd << 8));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[29], 0xffffff,
			      (pdram_timing->tfaw << 16) |
			      (pdram_timing->trppb << 8) | pdram_timing->twtr);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[35],
			 (pdram_timing->tcke << 24) | pdram_timing->tras_max);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[36], 0xff,
			      max(1, pdram_timing->tckesr));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[39],
			      (0xff << 24), (pdram_timing->trcd << 24));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[40],
			      0x3f, pdram_timing->twr);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[42], 0x1f << 24,
			      pdram_timing->tmrz << 24);
		tmp = pdram_timing->tdal ? pdram_timing->tdal :
		       (pdram_timing->twr + pdram_timing->trp);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[44], 0xff << 8,
			      tmp << 8);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[45], 0xff << 8,
			      pdram_timing->trp << 8);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[49],
			 ((pdram_timing->trefi - 8) << 16) |
			 pdram_timing->trfc);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[52], 0xffff << 16,
			      pdram_timing->txp << 16);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[54], 0xffff,
			      pdram_timing->txpdll);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[55], 0xff << 8,
			      pdram_timing->tmrri << 8);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[57],
			 (pdram_timing->tmrwckel << 24) |
			 (pdram_timing->tckehcs << 16) |
			 (pdram_timing->tckelcs << 8) | pdram_timing->tcscke);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[58], 0xf,
			      pdram_timing->tzqcke);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[61], 0xffff,
			      pdram_timing->txsnr);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[64], 0xffff << 16,
			      (pdram_timing->tckehcmd << 24) |
			      (pdram_timing->tckelcmd << 16));
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[65],
			 (pdram_timing->tckelpd << 24) |
			 (pdram_timing->tescke << 16) |
			 (pdram_timing->tsr << 8) | pdram_timing->tckckel);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[66], 0xfff,
			      (pdram_timing->tcmdcke << 8) |
			      pdram_timing->tcsckeh);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[92], (0xff << 24),
			      (pdram_timing->tcksre << 24));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[93], 0xff,
			      pdram_timing->tcksrx);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[108], (0x1 << 25),
			      (timing_config->dllbp << 25));
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[125],
			 (pdram_timing->tvrcg_disable << 16) |
			 pdram_timing->tvrcg_enable);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[126],
			 (pdram_timing->tckfspx << 24) |
			 (pdram_timing->tckfspe << 16) |
			 pdram_timing->tfc_long);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[127], 0xffff,
			      pdram_timing->tvref_long);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[134],
			      0xffff << 16, pdram_timing->mr[0] << 16);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[135],
			 (pdram_timing->mr[2] << 16) | pdram_timing->mr[1]);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[138],
			      0xffff << 16, pdram_timing->mr[3] << 16);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[140], 0xff,
			      pdram_timing->mr11);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[148],
			      0xffff << 16, pdram_timing->mr[0] << 16);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[149],
			 (pdram_timing->mr[2] << 16) | pdram_timing->mr[1]);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[152],
			      0xffff << 16, pdram_timing->mr[3] << 16);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[154], 0xff,
			      pdram_timing->mr11);
		if (timing_config->dram_type == LPDDR4) {
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[141],
				      0xffff, pdram_timing->mr12);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[143],
				      0xffff, pdram_timing->mr14);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[146],
				      0xffff, pdram_timing->mr22);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[155],
				      0xffff, pdram_timing->mr12);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[157],
				      0xffff, pdram_timing->mr14);
			clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[160],
				      0xffff, pdram_timing->mr22);
		}
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[182],
			 ((pdram_timing->tzqinit / 2) << 16) |
			 pdram_timing->tzqinit);
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[183],
			 (pdram_timing->tzqcal << 16) | pdram_timing->tzqcs);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[184], 0x3f,
			      pdram_timing->tzqlat);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[188], 0xfff,
			      pdram_timing->tzqreset);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[212], 0xff << 16,
			      pdram_timing->todton << 16);

		if (timing_config->odt) {
			setbits_32(&rk3399_ddr_pctl[i]->denali_ctl[213],
				   (1 << 24));
			if (timing_config->freq < 400)
				tmp = 4 << 24;
			else
				tmp = 8 << 24;
		} else {
			clrbits_32(&rk3399_ddr_pctl[i]->denali_ctl[213],
				   (1 << 24));
			tmp = 2 << 24;
		}
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[217], 0x1f << 24,
			      tmp);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[221], 0xf << 24,
			      (pdram_timing->tdqsck_max << 24));
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[222], 0x3,
			      pdram_timing->tdqsck);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[291], 0xffff,
			      (get_wrlat_adj(timing_config->dram_type,
					     pdram_timing->cwl) << 8) |
			      get_rdlat_adj(timing_config->dram_type,
					    pdram_timing->cl));

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[84], 0xffff,
			      (4 * pdram_timing->trefi) & 0xffff);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[84], 0xffff << 16,
			      ((2 * pdram_timing->trefi) & 0xffff) << 16);

		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			tmp = get_pi_wrlat(pdram_timing, timing_config);
			tmp1 = get_pi_todtoff_max(pdram_timing, timing_config);
			tmp = (tmp > tmp1) ? (tmp - tmp1) : 0;
		} else {
			tmp = 0;
		}
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[214], 0x3f << 24,
			      (tmp & 0x3f) << 24);

		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			/* min_rl_preamble= cl+TDQSCK_MIN-1 */
			tmp = pdram_timing->cl +
			    get_pi_todtoff_min(pdram_timing, timing_config) - 1;
			/* todtoff_max */
			tmp1 = get_pi_todtoff_max(pdram_timing, timing_config);
			tmp = (tmp > tmp1) ? (tmp - tmp1) : 0;
		} else {
			tmp = pdram_timing->cl - pdram_timing->cwl;
		}
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[215], 0x3f << 16,
			      (tmp & 0x3f) << 16);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[275], 0xff << 24,
			      (get_pi_tdfi_phy_rdlat
			       (pdram_timing, timing_config)
			       & 0xff) << 24);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[284],
			      0xffff << 16,
			      ((2 * pdram_timing->trefi) & 0xffff) << 16);

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[289], 0xffff,
			      (2 * pdram_timing->trefi) & 0xffff);

		write_32(&rk3399_ddr_pctl[i]->denali_ctl[290],
			 20 * pdram_timing->trefi);

		/* CTL_309 TDFI_CALVL_CAPTURE_F1:RW:16:10 */
		tmp1 = 20000 / (1000000 / pdram_timing->mhz) + 1;
		if ((20000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		tmp = (tmp1 >> 1) + (tmp1 % 2) + 5;
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[309], 0x3ff << 16,
				tmp << 16);

		/* CTL_309 TDFI_CALVL_CC_F1:RW:0:10 */
		tmp = tmp + 18;
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[309], 0x3ff,
				tmp);

		/* CTL_314 TDFI_WRCSLAT_F1:RW:24:8 */
		tmp1 = get_pi_wrlat_adj(pdram_timing, timing_config);
		if (timing_config->freq <= ENPER_CS_TRAINING_FREQ) {
			if (tmp1 < 5) {
				if (tmp1 == 0)
					tmp = 0;
				else
					tmp = tmp1 - 1;
			} else {
				tmp = tmp1 - 5;
			}
		} else {
			tmp = tmp1 - 2;
		}

		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[314], 0xff << 24,
				tmp << 24);

		/* CTL_314 TDFI_RDCSLAT_F1:RW:16:8 */
		if ((timing_config->freq <= ENPER_CS_TRAINING_FREQ) &&
			(pdram_timing->cl >= 5))
			tmp = pdram_timing->cl - 5;
		else
			tmp = pdram_timing->cl - 2;
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[314], 0xff << 16,
				tmp << 16);
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

#if CTL_TRAINING
	uint32_t i, tmp0, tmp1;

	tmp0 = tmp1 = 0;
#if EN_READ_GATE_TRAINING
	tmp1 = 1;
#endif

#if EN_CA_TRAINING
	tmp0 |= (1 << 8);
#endif

#if EN_WRITE_LEVELING
	tmp0 |= (1 << 16);
#endif

#if EN_READ_LEVELING
	tmp0 |= (1 << 24);
#endif
	for (i = 0; i < timing_config->ch_cnt; i++) {
		if (tmp0 | tmp1)
			setbits_32(&rk3399_ddr_pctl[i]->denali_ctl[305],
				   1 << 16);
		if (tmp0)
			setbits_32(&rk3399_ddr_pctl[i]->denali_ctl[70], tmp0);
		if (tmp1)
			setbits_32(&rk3399_ddr_pctl[i]->denali_ctl[71], tmp1);
	}
#endif
}

static void gen_rk3399_pi_params_f0(struct timing_related_config *timing_config,
				    struct dram_timing_t *pdram_timing)
{
	uint32_t tmp, tmp1, tmp2;
	uint32_t i;

	for (i = 0; i < timing_config->ch_cnt; i++) {
		/* PI_02 PI_TDFI_PHYMSTR_MAX_F0:RW:0:32 */
		tmp = 4 * pdram_timing->trefi;
		write_32(&rk3399_ddr_pi[i]->denali_pi[2], tmp);
		/* PI_03 PI_TDFI_PHYMSTR_RESP_F0:RW:0:16 */
		tmp = 2 * pdram_timing->trefi;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[3], 0xffff, tmp);
		/* PI_07 PI_TDFI_PHYUPD_RESP_F0:RW:16:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[7], 0xffff << 16,
			      tmp << 16);

		/* PI_42 PI_TDELAY_RDWR_2_BUS_IDLE_F0:RW:0:8 */
		if (timing_config->dram_type == LPDDR4)
			tmp = 2;
		else
			tmp = 0;
		tmp = (pdram_timing->bl / 2) + 4 +
		    (get_pi_rdlat_adj(pdram_timing) - 2) + tmp +
		    get_pi_tdfi_phy_rdlat(pdram_timing, timing_config);
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[42], 0xff, tmp);
		/* PI_43 PI_WRLAT_F0:RW:0:5 */
		if (timing_config->dram_type == LPDDR3) {
			tmp = get_pi_wrlat(pdram_timing, timing_config);
			clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[43], 0x1f,
				      tmp);
		}
		/* PI_43 PI_ADDITIVE_LAT_F0:RW:8:6 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[43], 0x3f << 8,
			      PI_ADD_LATENCY << 8);

		/* PI_43 PI_CASLAT_LIN_F0:RW:16:7 */
		tmp = pdram_timing->cl * 2;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[43], 0x7f << 16,
			      tmp << 16);
		/* PI_46 PI_TREF_F0:RW:16:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[46], 0xffff << 16,
			      pdram_timing->trefi << 16);
		/* PI_46 PI_TRFC_F0:RW:0:10 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[46], 0x3ff,
			      pdram_timing->trfc);
		/* PI_66 PI_TODTL_2CMD_F0:RW:24:8 */
		if (timing_config->dram_type == LPDDR3) {
			tmp = get_pi_todtoff_max(pdram_timing, timing_config);
			clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[66],
				      0xff << 24, tmp << 24);
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
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[72], 0x3f << 16,
			      tmp << 16);
		/* PI_73 PI_RD_TO_ODTH_F0:RW:8:6 */
		if ((timing_config->dram_type == LPDDR3) ||
		    (timing_config->dram_type == LPDDR4)) {
			/* min_rl_preamble= cl+TDQSCK_MIN-1 */
			tmp1 = pdram_timing->cl +
			    get_pi_todtoff_min(pdram_timing, timing_config) - 1;
			/* todtoff_max */
			tmp2 = get_pi_todtoff_max(pdram_timing, timing_config);
			if (tmp1 > tmp2)
				tmp = tmp1 - tmp2;
			else
				tmp = 0;
		} else if (timing_config->dram_type == DDR3) {
			tmp = pdram_timing->cl - pdram_timing->cwl;
		}
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[73], 0x3f << 8,
			      tmp << 8);
		/* PI_89 PI_RDLAT_ADJ_F0:RW:16:8 */
		tmp = get_pi_rdlat_adj(pdram_timing);
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[89], 0xff << 16,
			      tmp << 16);
		/* PI_90 PI_WRLAT_ADJ_F0:RW:16:8 */
		tmp = get_pi_wrlat_adj(pdram_timing, timing_config);
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[90], 0xff << 16,
			      tmp << 16);
		/* PI_91 PI_TDFI_WRCSLAT_F0:RW:16:8 */
		tmp1 = tmp;
		if (tmp1 < 5) {
			if (tmp1 == 0)
				tmp = 0;
			else
				tmp = tmp1 - 1;
		} else {
			tmp = tmp1 - 5;
		}
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[91], 0xff << 16,
			      tmp << 16);
		/* PI_95 PI_TDFI_CALVL_CAPTURE_F0:RW:16:10 */
		tmp1 = 20000 / (1000000 / pdram_timing->mhz) + 1;
		if ((20000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		tmp = (tmp1 >> 1) + (tmp1 % 2) + 5;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[95], 0x3ff << 16,
			      tmp << 16);
		/* PI_95 PI_TDFI_CALVL_CC_F0:RW:0:10 */
		tmp = tmp + 18;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[95], 0x3ff, tmp);
		/* PI_102 PI_TMRZ_F0:RW:8:5 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[102], 0x1f << 8,
			      pdram_timing->tmrz << 8);
		/* PI_111 PI_TDFI_CALVL_STROBE_F0:RW:8:4 */
		tmp1 = 2 * 1000 / (1000000 / pdram_timing->mhz);
		if ((2 * 1000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		/* pi_tdfi_calvl_strobe=tds_train+5 */
		tmp = tmp1 + 5;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[111], 0xf << 8,
			      tmp << 8);
		/* PI_116 PI_TCKEHDQS_F0:RW:16:6 */
		tmp = 10000 / (1000000 / pdram_timing->mhz);
		if ((10000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp++;
		if (pdram_timing->mhz <= 100)
			tmp = tmp + 1;
		else
			tmp = tmp + 8;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[116], 0x3f << 16,
			      tmp << 16);
		/* PI_125 PI_MR1_DATA_F0_0:RW+:8:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[125], 0xffff << 8,
			      pdram_timing->mr[1] << 8);
		/* PI_133 PI_MR1_DATA_F0_1:RW+:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[133], 0xffff,
			      pdram_timing->mr[1]);
		/* PI_140 PI_MR1_DATA_F0_2:RW+:16:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[140], 0xffff << 16,
			      pdram_timing->mr[1] << 16);
		/* PI_148 PI_MR1_DATA_F0_3:RW+:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[148], 0xffff,
			      pdram_timing->mr[1]);
		/* PI_126 PI_MR2_DATA_F0_0:RW+:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[126], 0xffff,
			      pdram_timing->mr[2]);
		/* PI_133 PI_MR2_DATA_F0_1:RW+:16:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[133], 0xffff << 16,
			      pdram_timing->mr[2] << 16);
		/* PI_141 PI_MR2_DATA_F0_2:RW+:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[141], 0xffff,
			      pdram_timing->mr[2]);
		/* PI_148 PI_MR2_DATA_F0_3:RW+:16:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[148], 0xffff << 16,
			      pdram_timing->mr[2] << 16);
		/* PI_156 PI_TFC_F0:RW:0:10 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[156], 0x3ff,
			      pdram_timing->trfc);
		/* PI_158 PI_TWR_F0:RW:24:6 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[158], 0x3f << 24,
			      pdram_timing->twr << 24);
		/* PI_158 PI_TWTR_F0:RW:16:6 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[158], 0x3f << 16,
			      pdram_timing->twtr << 16);
		/* PI_158 PI_TRCD_F0:RW:8:8 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[158], 0xff << 8,
			      pdram_timing->trcd << 8);
		/* PI_158 PI_TRP_F0:RW:0:8 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[158], 0xff,
			      pdram_timing->trp);
		/* PI_157 PI_TRTP_F0:RW:24:8 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[157], 0xff << 24,
			      pdram_timing->trtp << 24);
		/* PI_159 PI_TRAS_MIN_F0:RW:24:8 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[159], 0xff << 24,
			      pdram_timing->tras_min << 24);
		/* PI_159 PI_TRAS_MAX_F0:RW:0:17 */
		tmp = pdram_timing->tras_max * 99 / 100;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[159], 0x1ffff, tmp);
		/* PI_160 PI_TMRD_F0:RW:16:6 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[160], 0x3f << 16,
			      pdram_timing->tmrd << 16);
		/*PI_160 PI_TDQSCK_MAX_F0:RW:0:4 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[160], 0xf,
			      pdram_timing->tdqsck_max);
		/* PI_187 PI_TDFI_CTRLUPD_MAX_F0:RW:8:16 */
		tmp = 2 * pdram_timing->trefi;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[187], 0xffff << 8,
			      tmp << 8);
		/* PI_188 PI_TDFI_CTRLUPD_INTERVAL_F0:RW:0:32 */
		tmp = 20 * pdram_timing->trefi;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[188], 0xffffffff,
			      tmp);
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
		write_32(&rk3399_ddr_pi[i]->denali_pi[4], tmp);
		/* PI_05 PI_TDFI_PHYMSTR_RESP_F1:RW:0:16 */
		tmp = 2 * pdram_timing->trefi;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[5], 0xffff, tmp);
		/* PI_12 PI_TDFI_PHYUPD_RESP_F1:RW:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[12], 0xffff, tmp);

		/* PI_42 PI_TDELAY_RDWR_2_BUS_IDLE_F1:RW:8:8 */
		if (timing_config->dram_type == LPDDR4)
			tmp = 2;
		else
			tmp = 0;
		tmp = (pdram_timing->bl / 2) + 4 +
		    (get_pi_rdlat_adj(pdram_timing) - 2) + tmp +
		    get_pi_tdfi_phy_rdlat(pdram_timing, timing_config);
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[42], 0xff << 8,
			      tmp << 8);
		/* PI_43 PI_WRLAT_F1:RW:24:5 */
		if (timing_config->dram_type == LPDDR3) {
			tmp = get_pi_wrlat(pdram_timing, timing_config);
			clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[43],
				      0x1f << 24, tmp << 24);
		}
		/* PI_44 PI_ADDITIVE_LAT_F1:RW:0:6 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[44], 0x3f,
			      PI_ADD_LATENCY);
		/* PI_44 PI_CASLAT_LIN_F1:RW:8:7:=0x18 */
		tmp = pdram_timing->cl * 2;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[44], 0x7f << 8,
			      tmp << 8);
		/* PI_47 PI_TREF_F1:RW:16:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[47], 0xffff << 16,
			      pdram_timing->trefi << 16);
		/* PI_47 PI_TRFC_F1:RW:0:10 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[47], 0x3ff,
			      pdram_timing->trfc);
		/* PI_67 PI_TODTL_2CMD_F1:RW:8:8 */
		if (timing_config->dram_type == LPDDR3) {
			tmp = get_pi_todtoff_max(pdram_timing, timing_config);
			clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[67],
				      0xff << 8, tmp << 8);
		}
		/* PI_72 PI_WR_TO_ODTH_F1:RW:24:6 */
		if ((timing_config->dram_type == LPDDR3)
		    || (timing_config->dram_type == LPDDR4)) {
			tmp1 = get_pi_wrlat(pdram_timing, timing_config);
			tmp2 = get_pi_todtoff_max(pdram_timing, timing_config);
			if (tmp1 > tmp2)
				tmp = tmp1 - tmp2;
			else
				tmp = 0;
		} else if (timing_config->dram_type == DDR3) {
			tmp = 0;
		}
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[72], 0x3f << 24,
			      tmp << 24);
		/* PI_73 PI_RD_TO_ODTH_F1:RW:16:6 */
		if ((timing_config->dram_type == LPDDR3)
		    || (timing_config->dram_type == LPDDR4)) {
			/* min_rl_preamble= cl+TDQSCK_MIN-1 */
			tmp1 =
			    pdram_timing->cl + get_pi_todtoff_min(pdram_timing,
								  timing_config)
			    - 1;
			/* todtoff_max */
			tmp2 = get_pi_todtoff_max(pdram_timing, timing_config);
			if (tmp1 > tmp2)
				tmp = tmp1 - tmp2;
			else
				tmp = 0;
		} else if (timing_config->dram_type == DDR3) {
			tmp = pdram_timing->cl - pdram_timing->cwl;
		}
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[73], 0x3f << 16,
			      tmp << 16);
		/*P I_89 PI_RDLAT_ADJ_F1:RW:24:8 */
		tmp = get_pi_rdlat_adj(pdram_timing);
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[89], 0xff << 24,
			      tmp << 24);
		/* PI_90 PI_WRLAT_ADJ_F1:RW:24:8 */
		tmp = get_pi_wrlat_adj(pdram_timing, timing_config);
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[90], 0xff << 24,
			      tmp << 24);
		/* PI_91 PI_TDFI_WRCSLAT_F1:RW:24:8 */
		tmp1 = tmp;
		if (tmp1 < 5) {
			if (tmp1 == 0)
				tmp = 0;
			else
				tmp = tmp1 - 1;
		} else {
			tmp = tmp1 - 5;
		}
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[91], 0xff << 24,
			      tmp << 24);
		/*PI_96 PI_TDFI_CALVL_CAPTURE_F1:RW:16:10 */
		/* tadr=20ns */
		tmp1 = 20000 / (1000000 / pdram_timing->mhz) + 1;
		if ((20000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		tmp = (tmp1 >> 1) + (tmp1 % 2) + 5;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[96], 0x3ff << 16,
			      tmp << 16);
		/* PI_96 PI_TDFI_CALVL_CC_F1:RW:0:10 */
		tmp = tmp + 18;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[96], 0x3ff, tmp);
		/*PI_103 PI_TMRZ_F1:RW:0:5 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[103], 0x1f,
			      pdram_timing->tmrz);
		/*PI_111 PI_TDFI_CALVL_STROBE_F1:RW:16:4 */
		/* tds_train=ceil(2/ns) */
		tmp1 = 2 * 1000 / (1000000 / pdram_timing->mhz);
		if ((2 * 1000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp1++;
		/* pi_tdfi_calvl_strobe=tds_train+5 */
		tmp = tmp1 + 5;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[111], 0xf << 16,
			      tmp << 16);
		/* PI_116 PI_TCKEHDQS_F1:RW:24:6 */
		tmp = 10000 / (1000000 / pdram_timing->mhz);
		if ((10000 % (1000000 / pdram_timing->mhz)) != 0)
			tmp++;
		if (pdram_timing->mhz <= 100)
			tmp = tmp + 1;
		else
			tmp = tmp + 8;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[116], 0x3f << 24,
			      tmp << 24);
		/* PI_128 PI_MR1_DATA_F1_0:RW+:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[128], 0xffff,
			      pdram_timing->mr[1]);
		/* PI_135 PI_MR1_DATA_F1_1:RW+:8:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[135], 0xffff << 8,
			      pdram_timing->mr[1] << 8);
		/* PI_143 PI_MR1_DATA_F1_2:RW+:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[143], 0xffff,
			      pdram_timing->mr[1]);
		/* PI_150 PI_MR1_DATA_F1_3:RW+:8:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[150], 0xffff << 8,
			      pdram_timing->mr[1] << 8);
		/* PI_128 PI_MR2_DATA_F1_0:RW+:16:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[128], 0xffff << 16,
			      pdram_timing->mr[2] << 16);
		/* PI_136 PI_MR2_DATA_F1_1:RW+:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[136], 0xffff,
			      pdram_timing->mr[2]);
		/* PI_143 PI_MR2_DATA_F1_2:RW+:16:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[143], 0xffff << 16,
			      pdram_timing->mr[2] << 16);
		/* PI_151 PI_MR2_DATA_F1_3:RW+:0:16 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[151], 0xffff,
			      pdram_timing->mr[2]);
		/* PI_156 PI_TFC_F1:RW:16:10 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[156], 0x3ff << 16,
			      pdram_timing->trfc << 16);
		/* PI_162 PI_TWR_F1:RW:8:6 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[162], 0x3f << 8,
			      pdram_timing->twr << 8);
		/* PI_162 PI_TWTR_F1:RW:0:6 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[162], 0x3f,
			      pdram_timing->twtr);
		/* PI_161 PI_TRCD_F1:RW:24:8 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[161], 0xff << 24,
			      pdram_timing->trcd << 24);
		/* PI_161 PI_TRP_F1:RW:16:8 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[161], 0xff << 16,
			      pdram_timing->trp << 16);
		/* PI_161 PI_TRTP_F1:RW:8:8 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[161], 0xff << 8,
			      pdram_timing->trtp << 8);
		/* PI_163 PI_TRAS_MIN_F1:RW:24:8 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[163], 0xff << 24,
			      pdram_timing->tras_min << 24);
		/* PI_163 PI_TRAS_MAX_F1:RW:0:17 */
		tmp = pdram_timing->tras_max * 99 / 100;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[163], 0x1ffff, tmp);
		/* PI_164 PI_TMRD_F1:RW:16:6 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[164], 0x3f << 16,
			      pdram_timing->tmrd << 16);
		/* PI_164 PI_TDQSCK_MAX_F1:RW:0:4 */
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[164], 0xf,
			      pdram_timing->tdqsck_max);
		/* PI_189 PI_TDFI_CTRLUPD_MAX_F1:RW:0:16 */
		tmp = 2 * pdram_timing->trefi;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[189], 0xffff, tmp);
		/* PI_190 PI_TDFI_CTRLUPD_INTERVAL_F1:RW:0:32 */
		tmp = 20 * pdram_timing->trefi;
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[190], 0xffffffff,
			      tmp);
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

#if PI_TRAINING
		uint32_t i;

		for (i = 0; i < timing_config->ch_cnt; i++) {
#if EN_READ_GATE_TRAINING
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[80], 3 << 24,
			      2 << 24);
#endif

#if EN_CA_TRAINING
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[100], 3 << 8,
			      2 << 8);
#endif

#if EN_WRITE_LEVELING
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[60], 3 << 8,
			      2 << 8);
#endif

#if EN_READ_LEVELING
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[80], 3 << 16,
			      2 << 16);
#endif

#if EN_WDQ_LEVELING
		clrsetbits_32(&rk3399_ddr_pi[i]->denali_pi[124], 3 << 16,
			      2 << 16);
#endif
		}
#endif
}

static void gen_rk3399_set_odt(uint32_t odt_en)
{
	uint32_t drv_odt_val;
	uint32_t i;

	for (i = 0; i < rk3399_dram_status.timing_config.ch_cnt; i++) {
		drv_odt_val = (odt_en | (0 << 1) | (0 << 2)) << 16;
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[5],
				  0x7 << 16, drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[133],
				  0x7 << 16, drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[261],
				  0x7 << 16, drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[389],
				  0x7 << 16, drv_odt_val);
		drv_odt_val = (odt_en | (0 << 1) | (0 << 2)) << 24;
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[6],
				  0x7 << 24, drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[134],
				  0x7 << 24, drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[262],
				  0x7 << 24, drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[390],
				  0x7 << 24, drv_odt_val);
	}
}

static void gen_rk3399_set_ds_odt(struct timing_related_config *timing_config,
				  struct drv_odt_lp_config *drv_config)
{
	uint32_t i, drv_odt_val;

	for (i = 0; i < timing_config->ch_cnt; i++) {
		if (timing_config->dram_type == LPDDR4)
			drv_odt_val = drv_config->phy_side_odt |
				(PHY_DRV_ODT_Hi_Z << 4) |
				(drv_config->phy_side_dq_drv << 8) |
				(drv_config->phy_side_dq_drv << 12);
		else if (timing_config->dram_type == LPDDR3)
			drv_odt_val = PHY_DRV_ODT_Hi_Z |
				(drv_config->phy_side_odt << 4) |
				(drv_config->phy_side_dq_drv << 8) |
				(drv_config->phy_side_dq_drv << 12);
		else
			drv_odt_val = drv_config->phy_side_odt |
				(drv_config->phy_side_odt << 4) |
				(drv_config->phy_side_dq_drv << 8) |
				(drv_config->phy_side_dq_drv << 12);

		/* DQ drv odt set */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[6], 0xffffff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[134], 0xffffff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[262], 0xffffff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[390], 0xffffff,
				  drv_odt_val);
		/* DQS drv odt set */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[7], 0xffffff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[135], 0xffffff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[263], 0xffffff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[391], 0xffffff,
				  drv_odt_val);

		gen_rk3399_set_odt(timing_config->odt);

		/* CA drv set */
		drv_odt_val = drv_config->phy_side_ca_drv |
			(drv_config->phy_side_ca_drv << 4);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[544], 0xff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[672], 0xff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[800], 0xff,
				  drv_odt_val);

		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[928], 0xff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[937], 0xff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[935], 0xff,
				  drv_odt_val);

		drv_odt_val = drv_config->phy_side_ck_cs_drv |
			(drv_config->phy_side_ck_cs_drv << 4);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[929], 0xff,
				  drv_odt_val);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[939], 0xff,
				  drv_odt_val);
	}
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

		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[896],
			      (0x3 << 8) | 1, fn << 8);

		/* PHY_LOW_FREQ_SEL */
		/* DENALI_PHY_913 1bit offset_0 */
		if (timing_config->freq > 400)
			clrbits_32(&rk3399_ddr_publ[i]->denali_phy[913], 1);
		else
			setbits_32(&rk3399_ddr_publ[i]->denali_phy[913], 1);

		/* PHY_RPTR_UPDATE_x */
		/* DENALI_PHY_87/215/343/471 4bit offset_16 */
		tmp = 2500 / (1000000 / pdram_timing->mhz) + 3;
		if ((2500 % (1000000 / pdram_timing->mhz)) != 0)
			tmp++;
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[87], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[215], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[343], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[471], 0xf << 16,
			      tmp << 16);

		/* PHY_PLL_CTRL */
		/* DENALI_PHY_911 13bits offset_0 */
		/* PHY_LP4_BOOT_PLL_CTRL */
		/* DENALI_PHY_919 13bits offset_0 */
		if (pdram_timing->mhz <= 150)
			tmp = 3;
		else if (pdram_timing->mhz <= 300)
			tmp = 2;
		else if (pdram_timing->mhz <= 600)
			tmp = 1;
		else
			tmp = 0;
		tmp = (1 << 12) | (tmp << 9) | (2 << 7) | (1 << 1);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[911], 0x1fff,
			      tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[919], 0x1fff,
			      tmp);

		/* PHY_PLL_CTRL_CA */
		/* DENALI_PHY_911 13bits offset_16 */
		/* PHY_LP4_BOOT_PLL_CTRL_CA */
		/* DENALI_PHY_919 13bits offset_16 */
		if (pdram_timing->mhz <= 150)
			tmp = 3;
		else if (pdram_timing->mhz <= 300)
			tmp = 2;
		else if (pdram_timing->mhz <= 600)
			tmp = 1;
		else
			tmp = 0;
		tmp = (tmp << 9) | (2 << 7) | (1 << 5) | (1 << 1);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[911],
			      0x1fff << 16, tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[919],
			      0x1fff << 16, tmp << 16);

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
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[922], 0xf << 24,
			      tmp << 24);
		/* PHY_CAL_CLK_SELECT_0:RW8:3 */
		div = pdram_timing->mhz / (2 * 20);
		for (j = 2, tmp = 1; j <= 128; j <<= 1, tmp++) {
			if (div < j)
				break;
		}
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[947], 0x7 << 8,
			      tmp << 8);
		setbits_32(&rk3399_ddr_publ[i]->denali_phy[927], (1 << 22));

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
		delay_frac_ps =
		    1000 * total_delay_ps / (1000000 / pdram_timing->mhz);
		gate_delay_ps = delay_frac_ps + 1000 - (trpre_min_ps / 2);
		gate_delay_frac_ps =
		    gate_delay_ps - gate_delay_ps / 1000 * 1000;
		tmp = gate_delay_frac_ps * 0x200 / 1000;
		/* PHY_RDDQS_GATE_BYPASS_SLAVE_DELAY */
		/* DENALI_PHY_2/130/258/386 10bits offset_0 */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[2], 0x2ff, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[130], 0x2ff, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[258], 0x2ff, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[386], 0x2ff, tmp);
		/* PHY_RDDQS_GATE_SLAVE_DELAY */
		/* DENALI_PHY_77/205/333/461 10bits offset_16 */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[77], 0x2ff << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[205], 0x2ff << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[333], 0x2ff << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[461], 0x2ff << 16,
			      tmp << 16);

		tmp = gate_delay_ps / 1000;
		/* PHY_LP4_BOOT_RDDQS_LATENCY_ADJUST */
		/* DENALI_PHY_10/138/266/394 4bit offset_0 */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[10], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[138], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[266], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[394], 0xf, tmp);
		/* PHY_RDDQS_LATENCY_ADJUST */
		/* DENALI_PHY_78/206/334/462 4bits offset_0 */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[78], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[206], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[334], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[462], 0xf, tmp);
		/* PHY_GTLVL_LAT_ADJ_START */
		/* DENALI_PHY_80/208/336/464 4bits offset_16 */
		tmp = delay_frac_ps / 1000;
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[80], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[208], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[336], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[464], 0xf << 16,
			      tmp << 16);

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
		if (rddata_en_ie_dly > (cas_lat - 1 - hs_offset)) {
			tmp = 0;
		} else {
			if ((delta == 2) || (delta == 1))
				tmp = rddata_en_ie_dly - 0 - extra_adder;
			else
				tmp = extra_adder;
		}
		/* PHY_LP4_BOOT_RDDATA_EN_TSEL_DLY */
		/* DENALI_PHY_9/137/265/393 4bit offset_16 */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[9], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[137], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[265], 0xf << 16,
			      tmp << 16);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[393], 0xf << 16,
			      tmp << 16);
		/* PHY_RDDATA_EN_TSEL_DLY */
		/* DENALI_PHY_86/214/342/470 4bit offset_0 */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[86], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[214], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[342], 0xf, tmp);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[470], 0xf, tmp);

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
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[9], 0xf << 8,
			      tmp << 8);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[137], 0xf << 8,
			      tmp << 8);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[265], 0xf << 8,
			      tmp << 8);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[393], 0xf << 8,
			      tmp << 8);
		/* PHY_RDDATA_EN_DLY */
		/* DENALI_PHY_85/213/341/469 4bit offset_24 */
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[85], 0xf << 24,
			      tmp << 24);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[213], 0xf << 24,
			      tmp << 24);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[341], 0xf << 24,
			      tmp << 24);
		clrsetbits_32(&rk3399_ddr_publ[i]->denali_phy[469], 0xf << 24,
			      tmp << 24);

		if (pdram_timing->mhz <= ENPER_CS_TRAINING_FREQ) {

			/*
			 * Note:Per-CS Training is not compatible at speeds
			 * under 533 MHz. If the PHY is running at a speed
			 * less than 533MHz, all phy_per_cs_training_en_X
			 * parameters must be cleared to 0.
			 */

			/*DENALI_PHY_84/212/340/468 1bit offset_16 */
			clrbits_32(&rk3399_ddr_publ[i]->denali_phy[84],
				   0x1 << 16);
			clrbits_32(&rk3399_ddr_publ[i]->denali_phy[212],
				   0x1 << 16);
			clrbits_32(&rk3399_ddr_publ[i]->denali_phy[340],
				   0x1 << 16);
			clrbits_32(&rk3399_ddr_publ[i]->denali_phy[468],
				   0x1 << 16);
		} else {
			setbits_32(&rk3399_ddr_publ[i]->denali_phy[84],
				   0x1 << 16);
			setbits_32(&rk3399_ddr_publ[i]->denali_phy[212],
				   0x1 << 16);
			setbits_32(&rk3399_ddr_publ[i]->denali_phy[340],
				   0x1 << 16);
			setbits_32(&rk3399_ddr_publ[i]->denali_phy[468],
				   0x1 << 16);
		}
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

uint32_t rkclk_prepare_pll_timing(unsigned int mhz)
{
	unsigned int refdiv, postdiv1, fbdiv, postdiv2;
	int index;

	index = to_get_clk_index(mhz);
	refdiv = dpll_rates_table[index].refdiv;
	fbdiv = dpll_rates_table[index].fbdiv;
	postdiv1 = dpll_rates_table[index].postdiv1;
	postdiv2 = dpll_rates_table[index].postdiv2;
	write_32(DCF_PARAM_ADDR + PARAM_DPLL_CON0, FBDIV(fbdiv));
	write_32(DCF_PARAM_ADDR + PARAM_DPLL_CON1, POSTDIV2(postdiv2) |
		 POSTDIV1(postdiv1) | REFDIV(refdiv));
	return (24 * fbdiv) / refdiv / postdiv1 / postdiv2;
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
	struct rk3399_ddr_pctl_regs *ddr_pctl_regs;
	uint32_t low_power = 0;
	uint32_t channel_mask;
	uint32_t channel;
	uint32_t tmp;

	channel_mask = (read_32(PMUGRF_BASE + PMUGRF_OSREG(2)) >> 28) & 0x3;
	for (channel = 0; channel < 2; channel++) {
		ddr_pctl_regs = rk3399_ddr_pctl[channel];
		if (!(channel_mask & (1 << channel)))
			continue;

		/* exit stdby mode */
		write_32(&rk3399_ddr_cic->cic_ctrl1,
			 (1 << (channel + 16)) | (0 << channel));
		/* exit external self-refresh */
		tmp = channel ? 12 : 8;
		low_power |= ((read_32(PMU_BASE + PMU_SFT_CON) >> tmp) & 0x1)
		    << (4 + 8 * channel);
		clrbits_32(PMU_BASE + PMU_SFT_CON, 1 << tmp);
		while (!(read_32(PMU_BASE + PMU_DDR_SREF_ST) &
				(1 << channel)))
			;
		/* exit auto low-power */
		clrbits_32(&ddr_pctl_regs->denali_ctl[101], 0x7);
		/* lp_cmd to exit */
		if (((read_32(&ddr_pctl_regs->denali_ctl[100]) >> 24) &
			      0x7f) != 0x40) {
			while (read_32(&ddr_pctl_regs->denali_ctl[200]) & 0x1)
				;
			clrsetbits_32(&ddr_pctl_regs->denali_ctl[93],
				      0xff << 24, 0x69 << 24);
			while (((read_32(&ddr_pctl_regs->denali_ctl[100]) >>
					  24) & 0x7f) != 0x40)
				;
		}
	}
	return low_power;
}

void resume_low_power(uint32_t low_power)
{
	struct rk3399_ddr_pctl_regs *ddr_pctl_regs;
	uint32_t channel_mask;
	uint32_t channel;
	uint32_t tmp;
	uint32_t val;

	channel_mask = (read_32(PMUGRF_BASE + PMUGRF_OSREG(2)) >> 28) & 0x3;
	for (channel = 0; channel < 2; channel++) {
		ddr_pctl_regs = rk3399_ddr_pctl[channel];
		if (!(channel_mask & (1 << channel)))
			continue;

		/* resume external self-refresh */
		tmp = channel ? 12 : 8;
		val = (low_power >> (4 + 8 * channel)) & 0x1;
		setbits_32(PMU_BASE + PMU_SFT_CON, val << tmp);
		/* resume auto low-power */
		val = (low_power >> (8 * channel)) & 0x7;
		setbits_32(&ddr_pctl_regs->denali_ctl[101], val);
		/* resume stdby mode */
		val = (low_power >> (3 + 8 * channel)) & 0x1;
		write_32(&rk3399_ddr_cic->cic_ctrl1,
			 (1 << (channel + 16)) | (val << channel));
	}
}

static void wait_dcf_done(void)
{
	while ((read_32(DCF_BASE + DCF_DCF_ISR) & (DCF_DONE)) == 0)
		continue;
}

void clr_dcf_irq(void)
{
	/* clear dcf irq status */
	mmio_write_32(DCF_BASE + DCF_DCF_ISR, DCF_TIMEOUT | DCF_ERR | DCF_DONE);
}

static void enable_dcf(uint32_t dcf_addr)
{
	/* config DCF start addr */
	write_32(DCF_BASE + DCF_DCF_ADDR, dcf_addr);
	/* wait dcf done */
	while (read_32(DCF_BASE + DCF_DCF_CTRL) & 1)
		continue;
	/* clear dcf irq status */
	write_32(DCF_BASE + DCF_DCF_ISR, DCF_TIMEOUT | DCF_ERR | DCF_DONE);
	/* DCF start */
	setbits_32(DCF_BASE + DCF_DCF_CTRL, DCF_START);
}

void dcf_code_init(void)
{
	memcpy((void *)DCF_START_ADDR, (void *)dcf_code, sizeof(dcf_code));
	/* set dcf master secure */
	write_32(SGRF_BASE + 0xe01c, ((0x3 << 0) << 16) | (0 << 0));
	write_32(DCF_BASE + DCF_DCF_TOSET, 0x80000000);
}

static void dcf_start(uint32_t freq, uint32_t index)
{
	write_32(CRU_BASE + CRU_SOFTRST_CON(10), (0x1 << (1 + 16)) | (1 << 1));
	write_32(CRU_BASE + CRU_SOFTRST_CON(11), (0x1 << (0 + 16)) | (1 << 0));
	write_32(DCF_PARAM_ADDR + PARAM_FREQ_SELECT, index << 4);

	write_32(DCF_PARAM_ADDR + PARAM_DRAM_FREQ, freq);

	rkclk_prepare_pll_timing(freq);
	udelay(10);
	write_32(CRU_BASE + CRU_SOFTRST_CON(10), (0x1 << (1 + 16)) | (0 << 1));
	write_32(CRU_BASE + CRU_SOFTRST_CON(11), (0x1 << (0 + 16)) | (0 << 0));
	udelay(10);
	enable_dcf(DCF_START_ADDR);
}

static void dram_low_power_config(struct drv_odt_lp_config *lp_config)
{
	uint32_t tmp, tmp1, i;
	uint32_t ch_cnt = rk3399_dram_status.timing_config.ch_cnt;
	uint32_t dram_type = rk3399_dram_status.timing_config.dram_type;
	uint32_t *low_power = &rk3399_dram_status.low_power_stat;

	if (dram_type == LPDDR4)
		tmp = (lp_config->srpd_lite_idle << 16) |
		      lp_config->pd_idle;
	else
		tmp = lp_config->pd_idle;

	if (dram_type == DDR3)
		tmp1 = (2 << 16) | (0x7 << 8) | 7;
	else
		tmp1 = (3 << 16) | (0x7 << 8) | 7;

	*low_power = 0;

	for (i = 0; i < ch_cnt; i++) {
		write_32(&rk3399_ddr_pctl[i]->denali_ctl[102], tmp);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[103], 0xffff,
			      (lp_config->sr_mc_gate_idle << 8) |
			      lp_config->sr_idle);
		clrsetbits_32(&rk3399_ddr_pctl[i]->denali_ctl[101],
			      0x70f0f, tmp1);
		*low_power |= (7 << (8 * i));
	}

	/* standby idle */
	write_32(&rk3399_ddr_cic->cic_idle_th, lp_config->standby_idle);
	write_32(&rk3399_ddr_cic->cic_cg_wait_th, 0x640008);

	if (ch_cnt == 2) {
		write_32(GRF_BASE + GRF_DDRC1_CON1,
			 (((0x1<<4) | (0x1<<5) | (0x1<<6) | (0x1<<7)) << 16) |
			 ((0x1<<4) | (0x0<<5) | (0x1<<6) | (0x1<<7)));
		if (lp_config->standby_idle) {
			tmp = 0x002a002a;
			*low_power |= (1 << 11);
		} else {
			tmp = 0;
		}
		write_32(&rk3399_ddr_cic->cic_ctrl1, tmp);
	}

	write_32(GRF_BASE + GRF_DDRC0_CON1,
		 (((0x1<<4) | (0x1<<5) | (0x1<<6) | (0x1<<7)) << 16) |
		 ((0x1<<4) | (0x0<<5) | (0x1<<6) | (0x1<<7)));
	if (lp_config->standby_idle) {
		tmp = 0x00150015;
		*low_power |= (1 << 3);
	} else {
		tmp = 0;
	}
	write_32(&rk3399_ddr_cic->cic_ctrl1, tmp);
}


static void dram_related_init(struct ddr_dts_config_timing *dts_timing)
{
	uint32_t trefi0, trefi1;
	uint32_t i;
	struct rk3399_sdram_config sdram_config;

	dcf_code_init();

	/* get sdram config for os reg */
	sdram_config_init(&sdram_config);
	drv_odt_lp_cfg_init(sdram_config.dramtype, dts_timing,
			    &rk3399_dram_status.drv_odt_lp_cfg);
	sdram_timing_cfg_init(&rk3399_dram_status.timing_config,
			      &sdram_config,
			      &rk3399_dram_status.drv_odt_lp_cfg);

	trefi0 = ((read_32(&rk3399_ddr_pctl[0]->denali_ctl[48]) >>
		   16) & 0xffff) + 8;
	trefi1 = ((read_32(&rk3399_ddr_pctl[0]->denali_ctl[49]) >>
		   16) & 0xffff) + 8;

	rk3399_dram_status.index_freq[0] = trefi0 * 10 / 39;
	rk3399_dram_status.index_freq[1] = trefi1 * 10 / 39;
	rk3399_dram_status.current_index =
	    (read_32(&rk3399_ddr_pctl[0]->denali_ctl[111])
	     >> 16) & 0x3;
	if (rk3399_dram_status.timing_config.dram_type == DDR3) {
		rk3399_dram_status.index_freq[0] /= 2;
		rk3399_dram_status.index_freq[1] /= 2;
	}
	rk3399_dram_status.index_freq[(rk3399_dram_status.current_index + 1)
				      & 0x1] = 0;

	/* disable all training by ctl and pi */
	for (i = 0; i < rk3399_dram_status.timing_config.ch_cnt; i++) {
		clrbits_32(&rk3399_ddr_pctl[i]->denali_ctl[70], (1 << 24) |
				(1 << 16) | (1 << 8) | 1);
		clrbits_32(&rk3399_ddr_pctl[i]->denali_ctl[71], 1);

		clrbits_32(&rk3399_ddr_pi[i]->denali_pi[60], 0x3 << 8);
		clrbits_32(&rk3399_ddr_pi[i]->denali_pi[80], (0x3 << 24) |
				(0x3 << 16));
		clrbits_32(&rk3399_ddr_pi[i]->denali_pi[100], 0x3 << 8);
		clrbits_32(&rk3399_ddr_pi[i]->denali_pi[124], 0x3 << 16);
	}

	/* init drv odt */
	if (rk3399_dram_status.index_freq[rk3399_dram_status.current_index] <
		rk3399_dram_status.drv_odt_lp_cfg.odt_dis_freq)
		rk3399_dram_status.timing_config.odt = 0;
	else
		rk3399_dram_status.timing_config.odt = 1;
	gen_rk3399_set_ds_odt(&rk3399_dram_status.timing_config,
			&rk3399_dram_status.drv_odt_lp_cfg);
	dram_low_power_config(&rk3399_dram_status.drv_odt_lp_cfg);
}

static uint32_t prepare_ddr_timing(uint32_t mhz)
{
	uint32_t index;
	struct dram_timing_t dram_timing;

	rk3399_dram_status.timing_config.freq = mhz;

	if (mhz < rk3399_dram_status.drv_odt_lp_cfg.ddr3_dll_dis_freq)
		rk3399_dram_status.timing_config.dllbp = 1;
	else
		rk3399_dram_status.timing_config.dllbp = 0;
	if (mhz < rk3399_dram_status.drv_odt_lp_cfg.odt_dis_freq) {
		rk3399_dram_status.timing_config.odt = 0;
	} else {
		rk3399_dram_status.timing_config.odt = 1;
		gen_rk3399_set_odt(1);
	}

	index = (rk3399_dram_status.current_index + 1) & 0x1;
	if (rk3399_dram_status.index_freq[index] == mhz)
		goto out;

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


out:
	return index;
}

void print_dram_status_info(void)
{
	uint32_t *p;
	uint32_t i;

	p = (uint32_t *) &rk3399_dram_status.timing_config;
	INFO("rk3399_dram_status.timing_config:\n");
	for (i = 0; i < sizeof(struct timing_related_config) / 4; i++)
		tf_printf("%u\n", p[i]);
	p = (uint32_t *) &rk3399_dram_status.drv_odt_lp_cfg;
	INFO("rk3399_dram_status.drv_odt_lp_cfg:\n");
	for (i = 0; i < sizeof(struct drv_odt_lp_config) / 4; i++)
		tf_printf("%u\n", p[i]);
}

uint32_t ddr_set_rate(uint32_t hz)
{
	uint32_t low_power, index;
	uint32_t mhz = hz / (1000 * 1000);

	if (mhz ==
	    rk3399_dram_status.index_freq[rk3399_dram_status.current_index])
		goto out;

	index = to_get_clk_index(mhz);
	mhz = dpll_rates_table[index].mhz;

	low_power = exit_low_power();
	index = prepare_ddr_timing(mhz);
	if (index > 1)
		goto out;

	dcf_start(mhz, index);
	wait_dcf_done();
	if (rk3399_dram_status.timing_config.odt == 0)
		gen_rk3399_set_odt(0);

	rk3399_dram_status.current_index = index;

	if (mhz < dts_parameter.auto_pd_dis_freq)
		low_power |= rk3399_dram_status.low_power_stat;

	resume_low_power(low_power);
out:
	return mhz;
}

uint32_t ddr_round_rate(uint32_t hz)
{
	int index;
	uint32_t mhz = hz / (1000 * 1000);

	index = to_get_clk_index(mhz);

	return dpll_rates_table[index].mhz * 1000 * 1000;
}

uint32_t dts_timing_receive(uint32_t timing, uint32_t index)
{
	uint32_t *p = (uint32_t *) &dts_parameter;
	static uint32_t receive_nums;

	if (index < (sizeof(dts_parameter) / sizeof(uint32_t) - 1)) {
		p[index] = (uint32_t)timing;
		receive_nums++;
	} else {
		dts_parameter.available = 0;
		return -1;
	}

	/* receive all parameter */
	if (receive_nums  == (sizeof(dts_parameter) / sizeof(uint32_t) - 1)) {
		dts_parameter.available = 1;
		receive_nums = 0;
	}

	return index;
}

void ddr_init(void)
{
	dram_related_init(&dts_parameter);
}
