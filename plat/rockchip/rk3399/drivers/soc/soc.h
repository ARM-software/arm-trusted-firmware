/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_H__
#define __SOC_H__

#include <utils.h>

#define GLB_SRST_FST_CFG_VAL	0xfdb9
#define GLB_SRST_SND_CFG_VAL	0xeca8

#define PMUCRU_PPLL_CON(n)		((n) * 4)
#define CRU_PLL_CON(pll_id, n)	((pll_id)  * 0x20 + (n) * 4)
#define PLL_MODE_MSK			0x03
#define PLL_MODE_SHIFT			0x08
#define PLL_BYPASS_MSK			0x01
#define PLL_BYPASS_SHIFT		0x01
#define PLL_PWRDN_MSK			0x01
#define PLL_PWRDN_SHIFT			0x0
#define PLL_BYPASS			BIT(1)
#define PLL_PWRDN			BIT(0)

#define NO_PLL_BYPASS			(0x00)
#define NO_PLL_PWRDN			(0x00)

#define FBDIV(n)		((0xfff << 16) | n)
#define POSTDIV2(n)		((0x7 << (12 + 16)) | (n << 12))
#define POSTDIV1(n)		((0x7 << (8 + 16)) | (n << 8))
#define REFDIV(n)		((0x3F << 16) | n)
#define PLL_LOCK(n)		((n >> 31) & 0x1)

#define PLL_SLOW_MODE			BITS_WITH_WMASK(SLOW_MODE,\
						PLL_MODE_MSK, PLL_MODE_SHIFT)

#define PLL_NOMAL_MODE			BITS_WITH_WMASK(NORMAL_MODE,\
						PLL_MODE_MSK, PLL_MODE_SHIFT)

#define PLL_BYPASS_MODE			BIT_WITH_WMSK(PLL_BYPASS_SHIFT)
#define PLL_NO_BYPASS_MODE		WMSK_BIT(PLL_BYPASS_SHIFT)

#define PLL_CON_COUNT			0x06
#define CRU_CLKSEL_COUNT		108
#define CRU_CLKSEL_CON(n)		(0x100 + (n) * 4)

#define PMUCRU_CLKSEL_CONUT		0x06
#define PMUCRU_CLKSEL_OFFSET		0x080
#define REG_SIZE			0x04
#define REG_SOC_WMSK			0xffff0000
#define CLK_GATE_MASK			0x01

#define PMUCRU_GATE_COUNT	0x03
#define CRU_GATE_COUNT		0x23
#define PMUCRU_GATE_CON(n)	(0x100 + (n) * 4)
#define CRU_GATE_CON(n)	(0x300 + (n) * 4)

enum plls_id {
	ALPLL_ID = 0,
	ABPLL_ID,
	DPLL_ID,
	CPLL_ID,
	GPLL_ID,
	NPLL_ID,
	VPLL_ID,
	PPLL_ID,
	END_PLL_ID,
};

#define CLST_L_CPUS_MSK (0xf)
#define CLST_B_CPUS_MSK (0x3)

enum pll_work_mode {
	SLOW_MODE = 0x00,
	NORMAL_MODE = 0x01,
	DEEP_SLOW_MODE = 0x02,
};

enum glb_sft_reset {
	PMU_RST_BY_FIRST_SFT,
	PMU_RST_BY_SECOND_SFT = BIT(2),
	PMU_RST_NOT_BY_SFT = BIT(3),
};

struct pll_div {
	uint32_t mhz;
	uint32_t refdiv;
	uint32_t fbdiv;
	uint32_t postdiv1;
	uint32_t postdiv2;
	uint32_t frac;
	uint32_t freq;
};

struct deepsleep_data_s {
	uint32_t plls_con[END_PLL_ID][PLL_CON_COUNT];
	uint32_t cru_gate_con[CRU_GATE_COUNT];
	uint32_t pmucru_gate_con[PMUCRU_GATE_COUNT];
};

/**************************************************
 * pmugrf reg, offset
 **************************************************/
#define PMUGRF_OSREG(n)		(0x300 + (n) * 4)

/**************************************************
 * DCF reg, offset
 **************************************************/
#define DCF_DCF_CTRL		0x0
#define DCF_DCF_ADDR		0x8
#define DCF_DCF_ISR		0xc
#define DCF_DCF_TOSET		0x14
#define DCF_DCF_TOCMD		0x18
#define DCF_DCF_CMD_CFG		0x1c

/* DCF_DCF_ISR */
#define DCF_TIMEOUT		(1 << 2)
#define DCF_ERR			(1 << 1)
#define	DCF_DONE		(1 << 0)

/* DCF_DCF_CTRL */
#define DCF_VOP_HW_EN		(1 << 2)
#define DCF_STOP		(1 << 1)
#define DCF_START		(1 << 0)

#define CYCL_24M_CNT_US(us)	(24 * us)
#define CYCL_24M_CNT_MS(ms)	(ms * CYCL_24M_CNT_US(1000))
#define CYCL_32K_CNT_MS(ms)	(ms * 32)

/**************************************************
 * cru reg, offset
 **************************************************/
#define CRU_SOFTRST_CON(n)	(0x400 + (n) * 4)

#define CRU_DMAC0_RST		BIT_WITH_WMSK(3)
 /* reset release*/
#define CRU_DMAC0_RST_RLS	WMSK_BIT(3)

#define CRU_DMAC1_RST		BIT_WITH_WMSK(4)
 /* reset release*/
#define CRU_DMAC1_RST_RLS	WMSK_BIT(4)

#define CRU_GLB_RST_CON		0x0510
#define CRU_GLB_SRST_FST	0x0500
#define CRU_GLB_SRST_SND	0x0504

#define CRU_CLKGATE_CON(n)	(0x300 + n * 4)
#define PCLK_GPIO2_GATE_SHIFT	3
#define PCLK_GPIO3_GATE_SHIFT	4
#define PCLK_GPIO4_GATE_SHIFT	5

/**************************************************
 * pmu cru reg, offset
 **************************************************/
#define CRU_PMU_RSTHOLD_CON(n)		(0x120 + n * 4)
/* reset hold*/
#define CRU_PMU_SGRF_RST_HOLD		BIT_WITH_WMSK(6)
/* reset hold release*/
#define CRU_PMU_SGRF_RST_RLS		WMSK_BIT(6)

#define CRU_PMU_WDTRST_MSK		(0x1 << 4)
#define CRU_PMU_WDTRST_EN		0x0

#define CRU_PMU_FIRST_SFTRST_MSK	(0x3 << 2)
#define CRU_PMU_FIRST_SFTRST_EN		0x0

#define CRU_PMU_CLKGATE_CON(n)		(0x100 + n * 4)
#define PCLK_GPIO0_GATE_SHIFT		3
#define PCLK_GPIO1_GATE_SHIFT		4

#define CPU_BOOT_ADDR_WMASK	0xffff0000
#define CPU_BOOT_ADDR_ALIGN	16

#define GRF_IOMUX_2BIT_MASK     0x3
#define GRF_IOMUX_GPIO          0x0

#define GRF_GPIO4C2_IOMUX_SHIFT         4
#define GRF_GPIO4C2_IOMUX_PWM           0x1
#define GRF_GPIO4C6_IOMUX_SHIFT         12
#define GRF_GPIO4C6_IOMUX_PWM           0x1

#define PWM_CNT(n)			(0x0000 + 0x10 * (n))
#define PWM_PERIOD_HPR(n)		(0x0004 + 0x10 * (n))
#define PWM_DUTY_LPR(n)			(0x0008 + 0x10 * (n))
#define PWM_CTRL(n)			(0x000c + 0x10 * (n))

#define PWM_DISABLE			(0 << 0)
#define PWM_ENABLE			(1 << 0)

/* grf reg offset */
#define GRF_DDRC0_CON0		0xe380
#define GRF_DDRC0_CON1		0xe384
#define GRF_DDRC1_CON0		0xe388
#define GRF_DDRC1_CON1		0xe38c
#define GRF_SOC_CON_BASE	0xe200
#define GRF_SOC_CON(n)		(GRF_SOC_CON_BASE + (n) * 4)

#define PMUCRU_CLKSEL_CON0	0x0080
#define PMUCRU_CLKGATE_CON2	0x0108
#define PMUCRU_SOFTRST_CON0	0x0110
#define PMUCRU_GATEDIS_CON0 0x0130
#define PMUCRU_SOFTRST_CON(n)   (PMUCRU_SOFTRST_CON0 + (n) * 4)

/*
 * When system reset in running state, we want the cpus to be reboot
 * from maskrom (system reboot),
 * the pmusgrf reset-hold bits needs to be released.
 * When system wake up from system deep suspend, some soc will be reset
 * when waked up,
 * we want the bootcpu to be reboot from pmusram,
 * the pmusgrf reset-hold bits needs to be held.
 */
static inline void pmu_sgrf_rst_hld_release(void)
{
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_SGRF_RST_RLS);
}

static inline void pmu_sgrf_rst_hld(void)
{
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_SGRF_RST_HOLD);
}

/* export related and operating SoC APIs */
void __dead2 soc_global_soft_reset(void);
void disable_dvfs_plls(void);
void disable_nodvfs_plls(void);
void enable_dvfs_plls(void);
void enable_nodvfs_plls(void);
void prepare_abpll_for_ddrctrl(void);
void restore_abpll(void);
void restore_dpll(void);
void clk_gate_con_save(void);
void clk_gate_con_disable(void);
void clk_gate_con_restore(void);

#endif /* __SOC_H__ */
