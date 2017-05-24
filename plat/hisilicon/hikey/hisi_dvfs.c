/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <hi6220.h>
#include <hi6553.h>
#include <hisi_sram_map.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>

#define ACPU_FREQ_MAX_NUM		5
#define	ACPU_OPP_NUM			7

#define ACPU_VALID_VOLTAGE_MAGIC	(0x5A5AC5C5)

#define ACPU_WAIT_TIMEOUT		(200)
#define ACPU_WAIT_FOR_WFI_TIMOUT	(2000)
#define ACPU_DFS_STATE_CNT		(0x10000)

struct acpu_dvfs_sram_stru {
	unsigned int magic;
	unsigned int support_freq_num;
	unsigned int support_freq_max;
	unsigned int start_prof;
	unsigned int vol[ACPU_OPP_NUM];
};

struct acpu_volt_cal_para {
	unsigned int freq;
	unsigned int ul_vol;
	unsigned int dl_vol;
	unsigned int core_ref_hpm;
};

struct ddr_volt_cal_para {
	unsigned int freq;
	unsigned int ul_vol;
	unsigned int dl_vol;
	unsigned int ddr_ref_hpm;
};

struct acpu_dvfs_opp_para {
	unsigned int freq;
	unsigned int acpu_clk_profile0;
	unsigned int acpu_clk_profile1;
	unsigned int acpu_vol_profile;
	unsigned int acpu_pll_freq;
	unsigned int acpu_pll_frac;
};

unsigned int efuse_acpu_freq[] = {
	1200000, 1250000, 1300000, 1350000,
	1400000, 1450000, 1500000, 1550000,
	1600000, 1650000, 1700000, 1750000,
	1800000, 1850000, 1900000, 1950000,
};

struct acpu_dvfs_opp_para hi6220_acpu_profile[] = {
	{ 208000,  0x61E5, 0x022, 0x3A, 0x5220102B, 0x05555555 },
	{ 432000,  0x10A6, 0x121, 0x3A, 0x5120102D, 0x10000005 },
	{ 729000,  0x2283, 0x100, 0x4A, 0x51101026, 0x10000005 },
	{ 960000,  0x1211, 0x100, 0x5B, 0x51101032, 0x10000005 },
	{ 1200000, 0x1211, 0x100, 0x6B, 0x5110207D, 0x10000005 },
	{ 1400000, 0x1211, 0x100, 0x6B, 0x51101049, 0x10000005 },
	{ 1500000, 0x1211, 0x100, 0x6B, 0x51101049, 0x10000005 },
};

struct acpu_dvfs_opp_para *acpu_dvfs_profile = hi6220_acpu_profile;
struct acpu_dvfs_sram_stru *acpu_dvfs_sram_buf =
	(struct acpu_dvfs_sram_stru *)MEMORY_AXI_ACPU_FREQ_VOL_ADDR;

static inline void write_reg_mask(uintptr_t addr,
				  uint32_t val, uint32_t mask)
{
	uint32_t reg;

	reg = mmio_read_32(addr);
	reg = (reg & ~(mask)) | val;
	mmio_write_32(addr, reg);
}

static inline uint32_t read_reg_mask(uintptr_t addr,
				     uint32_t mask, uint32_t offset)
{
	uint32_t reg;

	reg = mmio_read_32(addr);
	reg &= (mask << offset);
	return (reg >> offset);
}

static int acpu_dvfs_syspll_cfg(unsigned int prof_id)
{
	uint32_t reg0 = 0;
	uint32_t count = 0;
	uint32_t clk_div_status = 0;

	/*
	 * step 1:
	 *  - ACPUSYSPLLCFG.acpu_subsys_clk_div_sw = 0x3;
	 *  - ACPUSYSPLLCFG.acpu_syspll_clken_cfg = 0x1;
	 */
	write_reg_mask(PMCTRL_ACPUSYSPLLCFG, 0x3 << 12, 0x3 << 12);
	write_reg_mask(PMCTRL_ACPUSYSPLLCFG, 0x1 << 4,  0x1 << 4);

	/*
	 * step 2:
	 *  - ACPUSYSPLLCFG.acpu_syspll_div_cfg:
	 *     208MHz, set to 0x5;
	 *     500MHz, set to 0x2;
	 *     other opps set to 0x1
	 */
	if (prof_id == 0)
		write_reg_mask(PMCTRL_ACPUSYSPLLCFG, 0x5 << 0, 0x7 << 0);
	else if (prof_id == 1)
		write_reg_mask(PMCTRL_ACPUSYSPLLCFG, 0x2 << 0, 0x7 << 0);
	else
		write_reg_mask(PMCTRL_ACPUSYSPLLCFG, 0x1 << 0, 0x7 << 0);

	/*
	 * step 3:
	 *  - Polling ACPU_SC_CPU_STAT.clk_div_status_vd == 0x3;
	 *  - ACPU_SC_VD_CTRL.tune_en_dif = 0
	 *  - ACPU_SC_VD_CTRL.tune_en_int = 0
	 *  - PMCTRL_ACPUCLKDIV.acpu_ddr_clk_div_cfg = 0x1
	 *  - PMCTRL_ACPUPLLSEL.acpu_pllsw_cfg = 0x1
	 */
	clk_div_status = 0x3;
	do {
		reg0 = read_reg_mask(ACPU_SC_CPU_STAT, 0x3, 20);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: clk div status timeout!\n", __func__);
			return -1;
		}
	} while (clk_div_status != reg0);

	write_reg_mask(ACPU_SC_VD_CTRL, 0x0, (0x1 << 0) | (0x1 << 11));
	write_reg_mask(PMCTRL_ACPUCLKDIV, 0x1 << 8, 0x3 << 8);
	write_reg_mask(PMCTRL_ACPUPLLSEL, 0x1 << 0, 0x1 << 0);

	return 0;
}

static void acpu_dvfs_clk_div_cfg(unsigned int prof_id,
				  unsigned int *cpuext_cfg,
				  unsigned int *acpu_ddr_cfg)
{
	if (prof_id == 0) {
		write_reg_mask(PMCTRL_ACPUCLKDIV,
			(0x1 << SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_cfg_START) |
			(0x1 << SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_cfg_START),
			(0x3 << SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_cfg_START) |
			(0x3 << SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_cfg_START));
		*cpuext_cfg = 0x1;
		*acpu_ddr_cfg = 0x1;
	} else if (prof_id == 1) {
		write_reg_mask(PMCTRL_ACPUCLKDIV,
			(0x1 << SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_cfg_START) |
			(0x1 << SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_cfg_START),
			(0x3 << SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_cfg_START) |
			(0x3 << SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_cfg_START));
		*cpuext_cfg = 0x1;
		*acpu_ddr_cfg = 0x1;
	} else {
		/* ddr has not been inited */
		write_reg_mask(PMCTRL_ACPUCLKDIV,
			(0x1 << SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_cfg_START) |
			(0x0 << SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_cfg_START),
			(0x3 << SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_cfg_START) |
			(0x3 << SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_cfg_START));
		*cpuext_cfg = 0x1;
		*acpu_ddr_cfg = 0x0;
	}
}

static int acpu_dvfs_freq_ascend(unsigned int cur_prof, unsigned int tar_prof)
{
	unsigned int reg0 = 0;
	unsigned int reg1 = 0;
	unsigned int reg2 = 0;
	unsigned int count = 0;
	unsigned int cpuext_cfg_val = 0;
	unsigned int acpu_ddr_cfg_val = 0;
	int ret = 0;

	/*
	 * step 1:
	 *  - PMCTRL_ACPUSYSPLLCFG.acpu_subsys_clk_div_sw = 0x3;
	 *  - ACPUSYSPLLCFG.acpu_syspll_clken_cfg = 0x1;
	 *
	 * step 2:
	 *  - PMCTRL_ACPUSYSPLLCFG.acpu_syspll_div_cfg = 0x5 (208MHz)
	 *  - PMCTRL_ACPUSYSPLLCFG.acpu_syspll_div_cfg = 0x2 (500MHz)
	 *  - PMCTRL_ACPUSYSPLLCFG.acpu_syspll_div_cfg = 0x1 (Other OPPs)
	 *
	 * step 3:
	 *  - ACPU_SC_CPU_STAT.clk_div_status_vd = 0x3;
	 *  - ACPU_SC_VD_CTRL.tune_en_dif = 0x0;
	 *  - ACPU_SC_VD_CTRL.tune_en_int = 0x0;
	 *  - PMCTRL_ACPUCLKDIV.acpu_ddr_clk_div_cfg = 0x1;
	 *  - PMCTRL_ACPUPLLSEL.acpu_pllsw_cfg = 0x1
	 */
	ret = acpu_dvfs_syspll_cfg(cur_prof);
	if (ret)
		return -1;

	/*
	 * step 4:
	 *  - Polling PMCTRL_ACPUPLLSEL.syspll_sw_stat == 0x1
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUPLLSEL, 0x1,
			SOC_PMCTRL_ACPUPLLSEL_syspll_sw_stat_START);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: syspll sw status timeout\n", __func__);
			return -1;
		}
	} while (reg0 != 0x1);

	/* Enable VD functionality if > 800MHz */
	if (acpu_dvfs_profile[tar_prof].freq > 800000) {

		write_reg_mask(ACPU_SC_VD_HPM_CTRL,
			HPM_OSC_DIV_VAL, HPM_OSC_DIV_MASK);

		/*
		 * step 5:
		 *  - ACPU_SC_VD_HPM_CTRL.hpm_dly_exp = 0xC7A;
		 *  - ACPU_SC_VD_MASK_PATTERN_CTRL[12:0] = 0xCCB;
		 */
		write_reg_mask(ACPU_SC_VD_HPM_CTRL,
			HPM_DLY_EXP_VAL, HPM_DLY_EXP_MASK);
		write_reg_mask(ACPU_SC_VD_MASK_PATTERN_CTRL,
			ACPU_SC_VD_MASK_PATTERN_VAL,
			ACPU_SC_VD_MASK_PATTERN_MASK);

		/*
		 * step 6:
		 *  - ACPU_SC_VD_DLY_TABLE0_CTRL = 0x1FFF;
		 *  - ACPU_SC_VD_DLY_TABLE1_CTRL = 0x1FFFFFF;
		 *  - ACPU_SC_VD_DLY_TABLE2_CTRL = 0x7FFFFFFF;
		 *  - ACPU_SC_VD_DLY_FIXED_CTRL  = 0x1;
		 */
		mmio_write_32(ACPU_SC_VD_DLY_TABLE0_CTRL, 0x1FFF);
		mmio_write_32(ACPU_SC_VD_DLY_TABLE1_CTRL, 0x1FFFFFF);
		mmio_write_32(ACPU_SC_VD_DLY_TABLE2_CTRL, 0x7FFFFFFF);
		mmio_write_32(ACPU_SC_VD_DLY_FIXED_CTRL, 0x1);

		/*
		 * step 7:
		 *  - ACPU_SC_VD_CTRL.shift_table0 = 0x1;
		 *  - ACPU_SC_VD_CTRL.shift_table1 = 0x3;
		 *  - ACPU_SC_VD_CTRL.shift_table2 = 0x5;
		 *  - ACPU_SC_VD_CTRL.shift_table3 = 0x6;
		 *
		 * step 8:
		 *  - ACPU_SC_VD_CTRL.tune = 0x7;
		 */
		write_reg_mask(ACPU_SC_VD_CTRL,
			ACPU_SC_VD_SHIFT_TABLE_TUNE_VAL,
			ACPU_SC_VD_SHIFT_TABLE_TUNE_MASK);
	}

	/* step 9: ACPUPLLCTRL.acpupll_en_cfg = 0x0 */
	write_reg_mask(PMCTRL_ACPUPLLCTRL, 0x0,
		0x1 << SOC_PMCTRL_ACPUPLLCTRL_acpupll_en_cfg_START);

	/* step 10: set PMCTRL_ACPUPLLFREQ and PMCTRL_ACPUPLLFRAC */
	mmio_write_32(PMCTRL_ACPUPLLFREQ,
		acpu_dvfs_profile[tar_prof].acpu_pll_freq);
	mmio_write_32(PMCTRL_ACPUPLLFRAC,
		acpu_dvfs_profile[tar_prof].acpu_pll_frac);

	/*
	 * step 11:
	 *  - wait for 1us;
	 *  - PMCTRL_ACPUPLLCTRL.acpupll_en_cfg = 0x1
	 */
	count = 0;
	while (count < ACPU_WAIT_TIMEOUT)
		count++;

	write_reg_mask(PMCTRL_ACPUPLLCTRL,
		0x1 << SOC_PMCTRL_ACPUPLLCTRL_acpupll_en_cfg_START,
		0x1 << SOC_PMCTRL_ACPUPLLCTRL_acpupll_en_cfg_START);

	/* step 12: PMCTRL_ACPUVOLPMUADDR = 0x100da */
	mmio_write_32(PMCTRL_ACPUVOLPMUADDR, 0x100da);

	/*
	 * step 13:
	 *  - PMCTRL_ACPUDESTVOL.acpu_dest_vol = 0x13 (208MHz);
	 *  - PMCTRL_ACPUDESTVOL.acpu_dest_vol = 0x13 (500MHz);
	 *  - PMCTRL_ACPUDESTVOL.acpu_dest_vol = 0x20 (798MHz);
	 *  - PMCTRL_ACPUDESTVOL.acpu_dest_vol = 0x3A (1300MHz);
	 *  - PMCTRL_ACPUDESTVOL.acpu_dest_vol = 0x3A (1500MHz);
	 */
	write_reg_mask(PMCTRL_ACPUDESTVOL,
		acpu_dvfs_profile[tar_prof].acpu_vol_profile,
		((0x1 << (SOC_PMCTRL_ACPUDESTVOL_acpu_dest_vol_END + 1)) - 1));

	/*
	 * step 14:
	 *  - Polling PMCTRL_ACPUDESTVOL.acpu_vol_using == ACPUDESTVOL.acpu_dest_vol
	 *  - Polling ACPUVOLTIMEOUT.acpu_vol_timeout == 0x1
	 *  - Config PMCTRL_ACPUCLKDIV.acpu_ddr_clk_div_cfg
	 *  - Config ACPUCLKDIV.cpuext_clk_div_cfg;
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUDESTVOL, 0x7F,
			SOC_PMCTRL_ACPUDESTVOL_acpu_dest_vol_START);
		reg1 = read_reg_mask(PMCTRL_ACPUDESTVOL, 0x7F,
			SOC_PMCTRL_ACPUDESTVOL_acpu_vol_using_START);
		reg2 = read_reg_mask(PMCTRL_ACPUVOLTTIMEOUT, 0x1,
			SOC_PMCTRL_ACPUVOLTIMEOUT_acpu_vol_timeout_START);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: acpu destvol cfg timeout.\n", __func__);
			return -1;
		}
	} while ((reg0 != reg1) || (reg2 != 0x1));

	acpu_dvfs_clk_div_cfg(tar_prof, &cpuext_cfg_val, &acpu_ddr_cfg_val);

	/*
	 * step 15:
	 *  - Polling PMCTRL_ACPUCLKDIV.cpuext_clk_div_stat;
	 *  - Polling ACPUCLKDIV.acpu_ddr_clk_div_stat;
	 *  - ACPUPLLCTRL.acpupll_timeout = 0x1;
	 *  - PMCTRL_ACPUPLLSEL.acpu_pllsw_cfg = 0x0;
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUCLKDIV, 0x3,
			SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_stat_START);
		reg1 = read_reg_mask(PMCTRL_ACPUCLKDIV, 0x3,
			SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_stat_START);
		reg2 = read_reg_mask(PMCTRL_ACPUPLLCTRL, 0x1,
			SOC_PMCTRL_ACPUPLLCTRL_acpupll_timeout_START);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: acpu clk div cfg timeout.\n", __func__);
			return -1;
		}
	} while ((reg1 != cpuext_cfg_val) ||
		(reg0 != acpu_ddr_cfg_val) ||
		(reg2 != 0x1));

	write_reg_mask(PMCTRL_ACPUPLLSEL, 0x0,
		0x1 << SOC_PMCTRL_ACPUPLLSEL_acpu_pllsw_cfg_START);

	/*
	 * step 16:
	 *  - Polling PMCTRL_ACPUPLLSEL.acpupll_sw_stat == 0x1;
	 *  - ACPU_SC_VD_CTRL.force_clk_en = 0x0;
	 *  - ACPU_SC_VD_CTRL.clk_dis_cnt_en = 0x0;
	 *  - ACPU_SC_VD_CTRL.calibrate_en_ini = 0x0;
	 *  - ACPU_SC_VD_CTRL.calibrate_en_dif = 0x0;
	 *  - ACPU_SC_VD_CTRL.div_en_dif = 0x1;
	 *  - ACPU_SC_VD_CTRL.tune_en_int = 0x1;
	 *  - ACPU_SC_VD_CTRL.tune_en_dif = 0x1;
	 *  - PMCTRL_ACPUSYSPLLCFG.acpu_subsys_clk_div_sw = 0x0;
	 *  - ACPUSYSPLLCFG.acpu_syspll_clken_cfg = 0x0;
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUPLLSEL, 0x1,
			SOC_PMCTRL_ACPUPLLSEL_acpu_pllsw_stat_START);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: acpu pll sw status timeout.\n", __func__);
			return -1;
		}
	} while (reg0 != 0x1);

	if (acpu_dvfs_profile[tar_prof].freq > 800000)
		write_reg_mask(ACPU_SC_VD_CTRL,
			ACPU_SC_VD_EN_ASIC_VAL, ACPU_SC_VD_EN_MASK);

	write_reg_mask(PMCTRL_ACPUSYSPLLCFG, 0x0,
		(0x3 << SOC_PMCTRL_ACPUSYSPLLCFG_acpu_subsys_clk_div_sw_START) |
		(0x1 << SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_clken_cfg_START));

	return 0;
}

static int acpu_dvfs_freq_descend(unsigned int cur_prof, unsigned int tar_prof)
{
	unsigned int reg0 = 0;
	unsigned int reg1 = 0;
	unsigned int reg2 = 0;
	unsigned int count = 0;
	unsigned int cpuext_cfg_val = 0;
	unsigned int acpu_ddr_cfg_val = 0;
	int ret = 0;

	ret = acpu_dvfs_syspll_cfg(tar_prof);
	if (ret)
		return -1;

	/*
	 * step 4:
	 *  - Polling PMCTRL_ACPUPLLSEL.syspll_sw_stat == 0x1
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUPLLSEL, 0x1, 2);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: syspll sw status timeout.\n", __func__);
			return -1;
		}
	} while (reg0 != 0x1);

	/*
	 * Step 5:
	 *  - PMCTRL_ACPUPLLCTRL.acpupll_en_cfg = 0x0
	 */
	write_reg_mask(PMCTRL_ACPUPLLCTRL, 0x0, 0x1 << 0);

	/*
	 * step 6
	 *  - Config PMCTRL_ACPUPLLFREQ and ACPUPLLFRAC
	 */
	mmio_write_32(PMCTRL_ACPUPLLFREQ, acpu_dvfs_profile[tar_prof].acpu_pll_freq);
	mmio_write_32(PMCTRL_ACPUPLLFRAC, acpu_dvfs_profile[tar_prof].acpu_pll_frac);

	/*
	 * step 7:
	 *  - Wait 1us;
	 *  - Config PMCTRL_ACPUPLLCTRL.acpupll_en_cfg = 0x1
	 */
	count = 0;
	while (count < ACPU_WAIT_TIMEOUT)
		count++;

	write_reg_mask(PMCTRL_ACPUPLLCTRL,
		0x1 << SOC_PMCTRL_ACPUPLLCTRL_acpupll_en_cfg_START,
		0x1 << SOC_PMCTRL_ACPUPLLCTRL_acpupll_en_cfg_START);

	/* Enable VD functionality if > 800MHz */
	if (acpu_dvfs_profile[tar_prof].freq > 800000) {

		write_reg_mask(ACPU_SC_VD_HPM_CTRL,
			HPM_OSC_DIV_VAL, HPM_OSC_DIV_MASK);

		/*
		 * step 9:
		 *  - ACPU_SC_VD_HPM_CTRL.hpm_dly_exp = 0xC7A;
		 *  - ACPU_SC_VD_MASK_PATTERN_CTRL[12:0] = 0xCCB;
		 */
		write_reg_mask(ACPU_SC_VD_HPM_CTRL,
			HPM_DLY_EXP_VAL, HPM_DLY_EXP_MASK);
		write_reg_mask(ACPU_SC_VD_MASK_PATTERN_CTRL,
			ACPU_SC_VD_MASK_PATTERN_VAL,
			ACPU_SC_VD_MASK_PATTERN_MASK);

		/*
		 * step 10:
		 *  - ACPU_SC_VD_DLY_TABLE0_CTRL = 0x1FFF;
		 *  - ACPU_SC_VD_DLY_TABLE1_CTRL = 0x1FFFFFF;
		 *  - ACPU_SC_VD_DLY_TABLE2_CTRL = 0x7FFFFFFF;
		 *  - ACPU_SC_VD_DLY_FIXED_CTRL  = 0x1;
		 */
		mmio_write_32(ACPU_SC_VD_DLY_TABLE0_CTRL, 0x1FFF);
		mmio_write_32(ACPU_SC_VD_DLY_TABLE1_CTRL, 0x1FFFFFF);
		mmio_write_32(ACPU_SC_VD_DLY_TABLE2_CTRL, 0x7FFFFFFF);
		mmio_write_32(ACPU_SC_VD_DLY_FIXED_CTRL, 0x1);

		/*
		 * step 11:
		 *  - ACPU_SC_VD_CTRL.shift_table0 = 0x1;
		 *  - ACPU_SC_VD_CTRL.shift_table1 = 0x3;
		 *  - ACPU_SC_VD_CTRL.shift_table2 = 0x5;
		 *  - ACPU_SC_VD_CTRL.shift_table3 = 0x6;
		 *
		 * step 12:
		 *  - ACPU_SC_VD_CTRL.tune = 0x7;
		 */
		write_reg_mask(ACPU_SC_VD_CTRL,
			ACPU_SC_VD_SHIFT_TABLE_TUNE_VAL,
			ACPU_SC_VD_SHIFT_TABLE_TUNE_MASK);
	}

	/*
	 * step 13:
	 *  - Pollig PMCTRL_ACPUPLLCTRL.acpupll_timeout == 0x1;
	 *  - PMCTRL_ACPUPLLSEL.acpu_pllsw_cfg = 0x0;
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUPLLCTRL, 0x1,
			SOC_PMCTRL_ACPUPLLCTRL_acpupll_timeout_START);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: acpupll timeout.\n", __func__);
			return -1;
		}
	} while (reg0 != 0x1);

	write_reg_mask(PMCTRL_ACPUPLLSEL, 0x0,
		0x1 << SOC_PMCTRL_ACPUPLLSEL_acpu_pllsw_cfg_START);

	/*
	 * step 14:
	 *  - Polling PMCTRL_ACPUPLLSEL.acpupll_sw_stat == 0x1;
	 *  - ACPU_SC_VD_CTRL.force_clk_en = 0x0;
	 *  - ACPU_SC_VD_CTRL.clk_dis_cnt_en = 0x0;
	 *  - ACPU_SC_VD_CTRL.calibrate_en_ini = 0x0;
	 *  - ACPU_SC_VD_CTRL.calibrate_en_dif = 0x0;
	 *  - ACPU_SC_VD_CTRL.div_en_dif = 0x1;
	 *  - ACPU_SC_VD_CTRL.tune_en_int = 0x1;
	 *  - ACPU_SC_VD_CTRL.tune_en_dif = 0x1;
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUPLLSEL, 0x1,
			SOC_PMCTRL_ACPUPLLSEL_acpu_pllsw_stat_START);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: acpupll sw status timeout.\n", __func__);
			return -1;
		}
	} while (reg0 != 0x1);

	if (acpu_dvfs_profile[tar_prof].freq > 800000)
		write_reg_mask(ACPU_SC_VD_CTRL,
			ACPU_SC_VD_EN_ASIC_VAL, ACPU_SC_VD_EN_MASK);

	/*
	 * step 15:
	 *  - PMCTRL_ACPUSYSPLLCFG.acpu_subsys_clk_div_sw = 0x0;
	 *  - ACPUSYSPLLCFG.acpu_syspll_clken_cfg = 0x0;
	 */
	write_reg_mask(PMCTRL_ACPUSYSPLLCFG, 0x0,
		(0x3 << SOC_PMCTRL_ACPUSYSPLLCFG_acpu_subsys_clk_div_sw_START) |
		(0x1 << SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_clken_cfg_START));

	/*
	 * step 16:
	 *  - Polling ACPU_SC_CPU_STAT.clk_div_status_vd == 0x0;
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(ACPU_SC_CPU_STAT, 0x3,
			ACPU_SC_CPU_STAT_CLK_DIV_STATUS_VD_SHIFT);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: clk div status timeout.\n", __func__);
			return -1;
		}
	} while (reg0 != 0x0);

	acpu_dvfs_clk_div_cfg(tar_prof, &cpuext_cfg_val, &acpu_ddr_cfg_val);

	/*
	 * step 17:
	 *  - Polling PMCTRL_ACPUCLKDIV.cpuext_clk_div_stat;
	 *  - Polling ACPUCLKDIV.acpu_ddr_clk_div_stat;
	 *  - PMCTRL_ACPUVOLPMUADDR = 0x1006C;
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUCLKDIV, 0x3,
			SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_stat_START);
		reg1 = read_reg_mask(PMCTRL_ACPUCLKDIV, 0x3,
			SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_stat_START);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: acpu clk div cfg timeout.\n", __func__);
			return -1;
		}
	} while ((reg0 != cpuext_cfg_val) || (reg1 != acpu_ddr_cfg_val));

	mmio_write_32(PMCTRL_ACPUVOLPMUADDR, 0x100da);

	/*
	 * step 16:
	 *  - Polling PMCTRL_ACPUPLLSEL.acpupll_sw_stat == 0x1;
	 *  - ACPU_SC_VD_CTRL.force_clk_en = 0x0;
	 *  - ACPU_SC_VD_CTRL.clk_dis_cnt_en = 0x0;
	 *  - ACPU_SC_VD_CTRL.calibrate_en_ini = 0x0;
	 *  - ACPU_SC_VD_CTRL.calibrate_en_dif = 0x0;
	 *  - ACPU_SC_VD_CTRL.div_en_dif = 0x1;
	 *  - ACPU_SC_VD_CTRL.tune_en_int = 0x1;
	 *  - ACPU_SC_VD_CTRL.tune_en_dif = 0x1;
	 *  - PMCTRL_ACPUSYSPLLCFG.acpu_subsys_clk_div_sw = 0x0;
	 *  - ACPUSYSPLLCFG.acpu_syspll_clken_cfg = 0x0;
	 */
	write_reg_mask(PMCTRL_ACPUDESTVOL,
		acpu_dvfs_profile[tar_prof].acpu_vol_profile,
		((0x1 << (SOC_PMCTRL_ACPUDESTVOL_acpu_dest_vol_END + 1)) - 1));

	/*
	 * step 19:
	 *  - Polling PMCTRL_ACPUDESTVOL.acpu_vol_using == ACPUDESTVOL.acpu_dest_vol
	 *  - ACPUVOLTIMEOUT.acpu_vol_timeout = 0x1;
	 */
	count = 0;
	do {
		reg0 = read_reg_mask(PMCTRL_ACPUDESTVOL, 0x7F,
			SOC_PMCTRL_ACPUDESTVOL_acpu_dest_vol_START);
		reg1 = read_reg_mask(PMCTRL_ACPUDESTVOL, 0x7F,
			SOC_PMCTRL_ACPUDESTVOL_acpu_vol_using_START);
		reg2 = read_reg_mask(PMCTRL_ACPUVOLTTIMEOUT, 0x1,
			SOC_PMCTRL_ACPUVOLTIMEOUT_acpu_vol_timeout_START);
		if ((count++) > ACPU_DFS_STATE_CNT) {
			ERROR("%s: acpu destvol cfg timeout.\n", __func__);
			return -1;
		}
	} while ((reg0 != reg1) || (reg2 != 0x1));

	return 0;
}

int acpu_dvfs_target(unsigned int curr_prof, unsigned int target_prof)
{
	int ret = 0;

	if (curr_prof == target_prof) {
		INFO("%s: target_prof is equal curr_prof: is %d!\n",
			__func__, curr_prof);
		return 0;
	}

	if ((curr_prof >= ACPU_FREQ_MAX_NUM) ||
	    (target_prof >= ACPU_FREQ_MAX_NUM)) {
		INFO("%s: invalid parameter %d %d\n",
			__func__, curr_prof, target_prof);
		return -1;
	}

	if (target_prof > acpu_dvfs_sram_buf->support_freq_num)
		target_prof = acpu_dvfs_sram_buf->support_freq_num;

	if (target_prof < curr_prof)
		ret = acpu_dvfs_freq_descend(curr_prof, target_prof);
	else if (target_prof > curr_prof)
		ret = acpu_dvfs_freq_ascend(curr_prof, target_prof);

	if (ret) {
		ERROR("%s: acpu_dvfs_target failed!\n", __func__);
		return -1;
	}

	/* Complete acpu dvfs setting and set magic number */
	acpu_dvfs_sram_buf->start_prof = target_prof;
	acpu_dvfs_sram_buf->magic = ACPU_VALID_VOLTAGE_MAGIC;

	mmio_write_32(DDR_DFS_FREQ_ADDR, 800000);
	return 0;
}

static int acpu_dvfs_set_freq(void)
{
	unsigned int i;
	unsigned int curr_prof;
	unsigned int target_prof;
	unsigned int max_freq = 0;

	max_freq = acpu_dvfs_sram_buf->support_freq_max;

	for (i = 0; i < acpu_dvfs_sram_buf->support_freq_num; i++) {

		if (max_freq == hi6220_acpu_profile[i].freq) {
			target_prof = i;
			break;
		}
	}

	if (i == acpu_dvfs_sram_buf->support_freq_num) {
		ERROR("%s: cannot found max freq profile\n", __func__);
		return -1;
	}

	curr_prof = 0;
	target_prof = i;

	/* if max freq is 208MHz, do nothing */
	if (curr_prof == target_prof)
		return 0;

	if (acpu_dvfs_target(curr_prof, target_prof)) {
		ERROR("%s: set acpu freq failed!", __func__);
		return -1;
	}

	INFO("%s: support freq num is %d\n",
		__func__, acpu_dvfs_sram_buf->support_freq_num);
	INFO("%s: start prof is 0x%x\n",
		__func__,  acpu_dvfs_sram_buf->start_prof);
	INFO("%s: magic is 0x%x\n",
		__func__, acpu_dvfs_sram_buf->magic);
	INFO("%s: voltage:\n", __func__);
	for (i = 0; i < acpu_dvfs_sram_buf->support_freq_num; i++)
		INFO("  - %d: 0x%x\n", i, acpu_dvfs_sram_buf->vol[i]);

	NOTICE("%s: set acpu freq success!", __func__);
	return 0;
}

struct acpu_dvfs_volt_setting {
	unsigned int magic;
	unsigned int support_freq_num;
	unsigned int support_freq_max;
	unsigned int start_prof;
	unsigned int vol[7];
	unsigned int hmp_dly_threshold[7];
};

static void acpu_dvfs_volt_init(void)
{
	struct acpu_dvfs_volt_setting *volt;

	/*
	 * - set default voltage;
	 * - set pmu address;
	 * - set voltage up and down step;
	 * - set voltage stable time;
	 */
	mmio_write_32(PMCTRL_ACPUDFTVOL, 0x4a);
	mmio_write_32(PMCTRL_ACPUVOLPMUADDR, 0xda);
	mmio_write_32(PMCTRL_ACPUVOLUPSTEP, 0x1);
	mmio_write_32(PMCTRL_ACPUVOLDNSTEP, 0x1);
	mmio_write_32(PMCTRL_ACPUPMUVOLUPTIME, 0x60);
	mmio_write_32(PMCTRL_ACPUPMUVOLDNTIME, 0x60);
	mmio_write_32(PMCTRL_ACPUCLKOFFCFG, 0x1000);

	volt = (void *)MEMORY_AXI_ACPU_FREQ_VOL_ADDR;
	volt->magic = 0x5a5ac5c5;
	volt->support_freq_num = 5;
	volt->support_freq_max = 1200000;
	volt->start_prof = 4;
	volt->vol[0] = 0x49;
	volt->vol[1] = 0x49;
	volt->vol[2] = 0x50;
	volt->vol[3] = 0x60;
	volt->vol[4] = 0x78;
	volt->vol[5] = 0x78;
	volt->vol[6] = 0x78;

	volt->hmp_dly_threshold[0] = 0x0;
	volt->hmp_dly_threshold[1] = 0x0;
	volt->hmp_dly_threshold[2] = 0x0;
	volt->hmp_dly_threshold[3] = 0x0e8b0e45;
	volt->hmp_dly_threshold[4] = 0x10691023;
	volt->hmp_dly_threshold[5] = 0x10691023;
	volt->hmp_dly_threshold[6] = 0x10691023;

	INFO("%s: success!\n", __func__);
}

void init_acpu_dvfs(void)
{
	unsigned int i = 0;

	INFO("%s: pmic version %d\n", __func__,
	     mmio_read_8(HI6553_VERSION_REG));

	/* init parameters */
	mmio_write_32(ACPU_CHIP_MAX_FREQ, efuse_acpu_freq[8]);
	INFO("%s: ACPU_CHIP_MAX_FREQ=0x%x.\n",
		__func__, mmio_read_32(ACPU_CHIP_MAX_FREQ));

	/* set maximum support frequency to 1.2GHz */
	for (i = 0; i < ACPU_FREQ_MAX_NUM; i++)
		acpu_dvfs_sram_buf->vol[i] = hi6220_acpu_profile[i].acpu_vol_profile;

	acpu_dvfs_sram_buf->support_freq_num = ACPU_FREQ_MAX_NUM;
	acpu_dvfs_sram_buf->support_freq_max = 1200000;

	/* init acpu dvfs */
	acpu_dvfs_volt_init();
	acpu_dvfs_set_freq();
}
