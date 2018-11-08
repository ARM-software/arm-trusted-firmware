/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HI6220_REGS_PMCTRL_H
#define HI6220_REGS_PMCTRL_H

#define PMCTRL_BASE				0xF7032000

#define PMCTRL_ACPUPLLCTRL			(PMCTRL_BASE + 0x000)
#define PMCTRL_ACPUPLLFREQ			(PMCTRL_BASE + 0x004)
#define PMCTRL_DDRPLL1CTRL			(PMCTRL_BASE + 0x010)
#define PMCTRL_DDRPLL0CTRL			(PMCTRL_BASE + 0x030)
#define PMCTRL_MEDPLLCTRL			(PMCTRL_BASE + 0x038)
#define PMCTRL_ACPUPLLSEL			(PMCTRL_BASE + 0x100)
#define PMCTRL_ACPUCLKDIV			(PMCTRL_BASE + 0x104)
#define PMCTRL_ACPUSYSPLLCFG			(PMCTRL_BASE + 0x110)
#define PMCTRL_ACPUCLKOFFCFG			(PMCTRL_BASE + 0x114)
#define PMCTRL_ACPUPLLFRAC			(PMCTRL_BASE + 0x134)
#define PMCTRL_ACPUPMUVOLUPTIME			(PMCTRL_BASE + 0x360)
#define PMCTRL_ACPUPMUVOLDNTIME			(PMCTRL_BASE + 0x364)
#define PMCTRL_ACPUVOLPMUADDR			(PMCTRL_BASE + 0x368)
#define PMCTRL_ACPUVOLUPSTEP			(PMCTRL_BASE + 0x36c)
#define PMCTRL_ACPUVOLDNSTEP			(PMCTRL_BASE + 0x370)
#define PMCTRL_ACPUDFTVOL			(PMCTRL_BASE + 0x374)
#define PMCTRL_ACPUDESTVOL			(PMCTRL_BASE + 0x378)
#define PMCTRL_ACPUVOLTTIMEOUT			(PMCTRL_BASE + 0x37c)

#define PMCTRL_ACPUPLLCTRL_EN_CFG		(1 << 0)

#define PMCTRL_ACPUCLKDIV_CPUEXT_CFG_MASK	(3 << 0)
#define PMCTRL_ACPUCLKDIV_DDR_CFG_MASK		(3 << 8)
#define PMCTRL_ACPUCLKDIV_CPUEXT_STAT_MASK	(3 << 16)
#define PMCTRL_ACPUCLKDIV_DDR_STAT_MASK		(3 << 24)

#define PMCTRL_ACPUPLLSEL_ACPUPLL_CFG		(1 << 0)
#define PMCTRL_ACPUPLLSEL_ACPUPLL_STAT		(1 << 1)
#define PMCTRL_ACPUPLLSEL_SYSPLL_STAT		(1 << 2)

#define PMCTRL_ACPUSYSPLL_CLKDIV_CFG_MASK	0x7
#define PMCTRL_ACPUSYSPLL_CLKEN_CFG		(1 << 4)
#define PMCTRL_ACPUSYSPLL_CLKDIV_SW		(3 << 12)

#define PMCTRL_ACPUSYSPLLCFG_SYSPLL_CLKEN	(1 << 4)
#define PMCTRL_ACPUSYSPLLCFG_CLKDIV_MASK	(3 << 12)

#define PMCTRL_ACPUDESTVOL_DEST_VOL_MASK	0x7f
#define PMCTRL_ACPUDESTVOL_CURR_VOL_MASK	(0x7f << 8)

#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_en_cfg_START   (0)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_en_cfg_END     (0)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_rst_START      (2)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_rst_END        (2)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_time_START     (4)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_time_END       (27)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_timeout_START  (28)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_timeout_END    (28)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_lock_START     (29)
#define SOC_PMCTRL_ACPUPLLCTRL_acpupll_lock_END       (29)

#define SOC_PMCTRL_ACPUPLLFRAC_ADDR(base)   ((base) + (0x134))
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_subsys_clk_div_sw_START   (12)

#define SOC_PMCTRL_ACPUPLLSEL_acpu_pllsw_cfg_START   (0)
#define SOC_PMCTRL_ACPUPLLSEL_acpu_pllsw_cfg_END     (0)
#define SOC_PMCTRL_ACPUPLLSEL_acpu_pllsw_stat_START  (1)
#define SOC_PMCTRL_ACPUPLLSEL_acpu_pllsw_stat_END    (1)
#define SOC_PMCTRL_ACPUPLLSEL_syspll_sw_stat_START   (2)
#define SOC_PMCTRL_ACPUPLLSEL_syspll_sw_stat_END     (2)

#define SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_cfg_START     (0)
#define SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_cfg_END       (1)
#define SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_cfg_START   (8)
#define SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_cfg_END     (9)
#define SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_stat_START    (16)
#define SOC_PMCTRL_ACPUCLKDIV_cpuext_clk_div_stat_END      (17)
#define SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_stat_START  (24)
#define SOC_PMCTRL_ACPUCLKDIV_acpu_ddr_clk_div_stat_END    (25)

#define SOC_PMCTRL_ACPUDESTVOL_acpu_dest_vol_START   (0)
#define SOC_PMCTRL_ACPUDESTVOL_acpu_dest_vol_END     (6)
#define SOC_PMCTRL_ACPUDESTVOL_acpu_vol_using_START  (8)
#define SOC_PMCTRL_ACPUDESTVOL_acpu_vol_using_END    (14)

#define SOC_PMCTRL_ACPUVOLTIMEOUT_acpu_vol_timeout_START  (0)
#define SOC_PMCTRL_ACPUVOLTIMEOUT_acpu_vol_timeout_END    (0)

#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_div_cfg_START      (0)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_div_cfg_END        (2)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_clken_cfg_START    (4)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_clken_cfg_END      (4)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_subsys_clk_div_cfg_START  (8)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_subsys_clk_div_cfg_END    (9)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_div_stat_START     (16)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_div_stat_END       (19)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_clken_stat_START   (20)
#define SOC_PMCTRL_ACPUSYSPLLCFG_acpu_syspll_clken_stat_END     (20)

#endif /* HI6220_REGS_PMCTRL_H */
