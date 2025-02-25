// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <platform.h>
#include <platform_def.h>

#include <dmc_rk3576.h>
#include <rk3576_def.h>
#include <soc.h>

struct rk3576_dmc_config dmc_config;

/* DDR_PHY */
#define LP_CON0				0x0018
#define DFI_LP_CON0			0x0e04
/* DDR_CTL */
#define DDRCTL_STAT			0x10014
#define DDRCTL_PWRCTL			0x10180
#define DDRCTL_CLKGATECTL		0x1018c

/* LP_CON0 */
#define DS_IO_PD			BIT(14)
#define SCHD_HW_CLOCK_GATING_DISABLE	BIT(13)
#define PCL_PD				BIT(12)
#define DQS_ENABLE			BIT(10)
#define WCK_ENABLE			BIT(9)
#define CTRL_DQS_DRV_OFF		BIT(8)
#define CTRL_SCHEDULER_EN		BIT(6)

/* DFI_LP_CON0 0x0e04 */
#define DFI_LP_MODE_APB			BIT(31)

/* DDRCTL_STAT 0x10014 */
#define CTL_SELFREF_STATE_SHIFT		(12)
#define CTL_SELFREF_STATE_MASK		(0x7 << CTL_SELFREF_STATE_SHIFT)
#define CTL_NOT_IN_SELF_REFRESH		(0x0 << CTL_SELFREF_STATE_SHIFT)
#define CTL_SELF_REFRESH_1		(0x1 << CTL_SELFREF_STATE_SHIFT)
#define CTL_SELF_REFRESH_POWER_DOWN	(0x2 << CTL_SELFREF_STATE_SHIFT)
#define CTL_SELF_REFRESH_2		(0x3 << CTL_SELFREF_STATE_SHIFT)
#define CTL_SELF_REFRESH_DEEP_SLEEP	(0x4 << CTL_SELFREF_STATE_SHIFT)
#define CTL_SELFREF_TYPE_SHIFT		(4)
#define CTL_SELFREF_TYPE_MASK		(0x3 << CTL_SELFREF_TYPE_SHIFT)
#define CTL_SELFREF_NOT_BY_PHY		(0x1 << CTL_SELFREF_TYPE_SHIFT)
#define CTL_SELFREF_NOT_BY_AUTO		(0x2 << CTL_SELFREF_TYPE_SHIFT)
#define CTL_SELFREF_BY_AUTO		(0x3 << CTL_SELFREF_TYPE_SHIFT)
#define CTL_OPERATING_MODE_MASK		(0x7)
#define CTL_OPERATING_MODE_INIT		(0x0)
#define CTL_OPERATING_MODE_NORMAL	(0x1)
#define CTL_OPERATING_MODE_PD		(0x2)
#define CTL_OPERATING_MODE_SR_SRPD	(0x3)

/* DDRCTL_PWRCTL 0x10180 */
#define CTL_DSM_EN			BIT(18)
#define CTL_STAY_IN_SELFREF		BIT(15)
#define CTL_SELFREF_SW			BIT(11)
#define CTL_EN_DFI_DRAM_CLK_DISABLE	BIT(9)
#define CTL_POWERDOWN_EN_MASK		(0xf)
#define CTL_POWERDOWN_EN_SHIFT		(4)
#define CTL_SELFREF_EN_MASK		(0xf)
#define CTL_SELFREF_EN_SHIFT		(0)

#define SYS_REG_DEC_CHINFO(n, ch)	(((n) >> (28 + (ch))) & 0x1)
#define SYS_REG_DEC_CHINFO_V3(reg2, ch) SYS_REG_DEC_CHINFO(reg2, ch)

#define SYS_REG_DEC_NUM_CH(n)		(1 + (((n) >> 12) & 0x1))
#define SYS_REG_DEC_NUM_CH_V3(reg2)	SYS_REG_DEC_NUM_CH(reg2)

static void exit_low_power(uint32_t ch, struct rk3576_dmc_config *configs)
{
	/* LP_CON0: [12]pcl_pd */
	configs->low_power[ch].pcl_pd = mmio_read_32(DDRPHY_BASE_CH(0) + LP_CON0) & PCL_PD;
	mmio_clrbits_32(DDRPHY_BASE_CH(ch) + LP_CON0, PCL_PD);

	/* Disable low power activities */
	configs->low_power[ch].pwrctl = mmio_read_32(UMCTL_BASE_CH(ch) + DDRCTL_PWRCTL);
	mmio_clrbits_32(UMCTL_BASE_CH(ch) + DDRCTL_PWRCTL,
			CTL_DSM_EN | (CTL_POWERDOWN_EN_MASK << CTL_POWERDOWN_EN_SHIFT) |
			(CTL_SELFREF_EN_MASK << CTL_SELFREF_EN_SHIFT));
	while ((mmio_read_32(UMCTL_BASE_CH(ch) + DDRCTL_STAT) & CTL_OPERATING_MODE_MASK) !=
		CTL_OPERATING_MODE_NORMAL)
		continue;

	/* DDR_GRF_CHA_CON6: [6:0]rd_lat_delay, [14:8]wr_lat_delay, [15]cmd_dly_eq0_en */
	configs->low_power[ch].grf_ddr_con6 =
		mmio_read_32(DDR_GRF_BASE + GRF_CH_CON(ch, 6)) & 0xff7f;
	mmio_write_32(DDR_GRF_BASE + GRF_CH_CON(ch, 6), (0x1ul << (15 + 16)));

	/* DDR_GRF_CHA_CON0: [12:8]ddrctl_axi_cg_en */
	configs->low_power[ch].grf_ddr_con0 =
		mmio_read_32(DDR_GRF_BASE + GRF_CH_CON(ch, 0)) & 0x1f00;
	mmio_write_32(DDR_GRF_BASE + GRF_CH_CON(ch, 0), 0x1f000000);

	/*
	 * DDR_GRF_CHA_CON1:
	 * [15]ddrctl_apb_pclk_cg_en, [12]ddrmon_pclk_cg_en, [7]dfi_scramble_cg_en,
	 * [6]ddrctl_mem_cg_en, [5]bsm_clk_cg_en, [2]ddrctl_core_cg_en, [1]ddrctl_apb_cg_en
	 */
	configs->low_power[ch].grf_ddr_con1 =
		mmio_read_32(DDR_GRF_BASE + GRF_CH_CON(ch, 1)) & 0x90e6;
	mmio_write_32(DDR_GRF_BASE + GRF_CH_CON(ch, 1), 0x90e60000);

	configs->low_power[ch].hwlp_0 = mmio_read_32(HWLP_BASE_CH(ch) + 0x0);
	mmio_write_32(HWLP_BASE_CH(ch) + 0x0, 0x0);
	configs->low_power[ch].hwlp_c = mmio_read_32(HWLP_BASE_CH(ch) + 0xc);
	mmio_write_32(HWLP_BASE_CH(ch) + 0xc, 0x0);

	/* DDR_GRF_CHA_PHY_CON0: [14]ddrphy_pclk_cg_en */
	configs->low_power[ch].grf_ddrphy_con0 =
		mmio_read_32(DDR_GRF_BASE + GRF_DDRPHY_CON0(ch)) & BIT(14);
	mmio_write_32(DDR_GRF_BASE + GRF_DDRPHY_CON0(ch), BIT(14 + 16));

	/* CLKGATECTL: [5:0]bsm_clk_on */
	configs->low_power[ch].clkgatectl =
		mmio_read_32(UMCTL_BASE_CH(ch) + DDRCTL_CLKGATECTL) & 0x3f;
	/* DFI_LP_CON0: [31]dfi_lp_mode_apb */
	configs->low_power[ch].dfi_lp_mode_apb =
		(mmio_read_32(DDRPHY_BASE_CH(ch) + DFI_LP_CON0) >> 31) & 0x1;
}

static void resume_low_power(uint32_t ch, struct rk3576_dmc_config *configs)
{
	/* DFI_LP_CON0: [31]dfi_lp_mode_apb */
	if (configs->low_power[ch].dfi_lp_mode_apb != 0)
		mmio_setbits_32(DDRPHY_BASE_CH(ch) + DFI_LP_CON0, DFI_LP_MODE_APB);

	/* CLKGATECTL: [5:0]bsm_clk_on */
	mmio_clrsetbits_32(UMCTL_BASE_CH(ch) + DDRCTL_CLKGATECTL,
			   0x3f, configs->low_power[ch].clkgatectl & 0x3f);

	/* DDR_GRF_CHA_CON6: [6:0]rd_lat_delay, [14:8]wr_lat_delay, [15]cmd_dly_eq0_en */
	mmio_write_32(DDR_GRF_BASE + GRF_CH_CON(ch, 6),
		      (0xff7ful << 16) | configs->low_power[ch].grf_ddr_con6);

	mmio_write_32(HWLP_BASE_CH(ch) + 0xc, configs->low_power[ch].hwlp_c);
	mmio_write_32(HWLP_BASE_CH(ch) + 0x0, configs->low_power[ch].hwlp_0);

	/* DDR_GRF_CHA_CON0: [12:8]ddrctl_axi_cg_en */
	mmio_write_32(DDR_GRF_BASE + GRF_CH_CON(ch, 0),
		      (0x1f00ul << 16) | configs->low_power[ch].grf_ddr_con0);

	/*
	 * DDR_GRF_CHA_CON1:
	 * [15]ddrctl_apb_pclk_cg_en, [12]ddrmon_pclk_cg_en, [7]dfi_scramble_cg_en,
	 * [6]ddrctl_mem_cg_en, [5]bsm_clk_cg_en, [2]ddrctl_core_cg_en, [1]ddrctl_apb_cg_en
	 */
	mmio_write_32(DDR_GRF_BASE + GRF_CH_CON(ch, 1),
		      (0x90e6ul << 16) | configs->low_power[ch].grf_ddr_con1);

	/* DDR_GRF_CHA_PHY_CON0: [14]ddrphy_pclk_cg_en */
	mmio_write_32(DDR_GRF_BASE + GRF_DDRPHY_CON0(ch),
		      BIT(14 + 16) | configs->low_power[ch].grf_ddrphy_con0);

	/* reset low power activities */
	mmio_write_32(UMCTL_BASE_CH(ch) + DDRCTL_PWRCTL, configs->low_power[ch].pwrctl);

	/* LP_CON0: [12]pcl_pd */
	if (configs->low_power[ch].pcl_pd != 0)
		mmio_setbits_32(DDRPHY_BASE_CH(ch) + LP_CON0, PCL_PD);
}

void dmc_save(void)
{
	uint32_t i, channel_num;

	channel_num =
		SYS_REG_DEC_NUM_CH_V3(mmio_read_32(PMU1_GRF_BASE + PMUGRF_OS_REG(2)));

	for (i = 0; i < channel_num; i++)
		exit_low_power(i, &dmc_config);
}

void dmc_restore(void)
{
	uint32_t i, channel_num;

	channel_num = SYS_REG_DEC_NUM_CH_V3(mmio_read_32(PMU1_GRF_BASE + PMUGRF_OS_REG(2)));

	for (i = 0; i < channel_num; i++)
		resume_low_power(i, &dmc_config);
}
