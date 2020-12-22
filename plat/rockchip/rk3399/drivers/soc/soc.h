/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define SOC_H

#include <lib/utils.h>

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

#define PMUCRU_RSTNHOLD_CON0	0x120
enum {
	PRESETN_NOC_PMU_HOLD = 1,
	PRESETN_INTMEM_PMU_HOLD,
	HRESETN_CM0S_PMU_HOLD,
	HRESETN_CM0S_NOC_PMU_HOLD,
	DRESETN_CM0S_PMU_HOLD,
	POESETN_CM0S_PMU_HOLD,
	PRESETN_SPI3_HOLD,
	RESETN_SPI3_HOLD,
	PRESETN_TIMER_PMU_0_1_HOLD,
	RESETN_TIMER_PMU_0_HOLD,
	RESETN_TIMER_PMU_1_HOLD,
	PRESETN_UART_M0_PMU_HOLD,
	RESETN_UART_M0_PMU_HOLD,
	PRESETN_WDT_PMU_HOLD
};

#define PMUCRU_RSTNHOLD_CON1	0x124
enum {
	PRESETN_I2C0_HOLD,
	PRESETN_I2C4_HOLD,
	PRESETN_I2C8_HOLD,
	PRESETN_MAILBOX_PMU_HOLD,
	PRESETN_RKPWM_PMU_HOLD,
	PRESETN_PMUGRF_HOLD,
	PRESETN_SGRF_HOLD,
	PRESETN_GPIO0_HOLD,
	PRESETN_GPIO1_HOLD,
	PRESETN_CRU_PMU_HOLD,
	PRESETN_INTR_ARB_HOLD,
	PRESETN_PVTM_PMU_HOLD,
	RESETN_I2C0_HOLD,
	RESETN_I2C4_HOLD,
	RESETN_I2C8_HOLD
};

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

struct pmu_sleep_data {
	uint32_t pmucru_rstnhold_con0;
	uint32_t pmucru_rstnhold_con1;
};

/**************************************************
 * pmugrf reg, offset
 **************************************************/
#define PMUGRF_OSREG(n)		(0x300 + (n) * 4)
#define PMUGRF_GPIO0A_P		0x040
#define PMUGRF_GPIO1A_P		0x050

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
#define GRF_USBPHY0_CTRL0	0x4480
#define GRF_USBPHY0_CTRL2	0x4488
#define GRF_USBPHY0_CTRL3	0x448c
#define GRF_USBPHY0_CTRL12	0x44b0
#define GRF_USBPHY0_CTRL13	0x44b4
#define GRF_USBPHY0_CTRL15	0x44bc
#define GRF_USBPHY0_CTRL16	0x44c0

#define GRF_USBPHY1_CTRL0	0x4500
#define GRF_USBPHY1_CTRL2	0x4508
#define GRF_USBPHY1_CTRL3	0x450c
#define GRF_USBPHY1_CTRL12	0x4530
#define GRF_USBPHY1_CTRL13	0x4534
#define GRF_USBPHY1_CTRL15	0x453c
#define GRF_USBPHY1_CTRL16	0x4540

#define GRF_GPIO2A_IOMUX	0xe000
#define GRF_GPIO2A_P		0xe040
#define GRF_GPIO3A_P		0xe050
#define GRF_GPIO4A_P		0xe060
#define GRF_GPIO2D_HE		0xe18c
#define GRF_DDRC0_CON0		0xe380
#define GRF_DDRC0_CON1		0xe384
#define GRF_DDRC1_CON0		0xe388
#define GRF_DDRC1_CON1		0xe38c
#define GRF_SOC_CON_BASE	0xe200
#define GRF_SOC_CON(n)		(GRF_SOC_CON_BASE + (n) * 4)
#define GRF_IO_VSEL		0xe640

#define CRU_CLKSEL_CON0		0x0100
#define CRU_CLKSEL_CON6		0x0118
#define CRU_SDIO0_CON1		0x058c
#define PMUCRU_CLKSEL_CON0	0x0080
#define PMUCRU_CLKGATE_CON2	0x0108
#define PMUCRU_SOFTRST_CON0	0x0110
#define PMUCRU_GATEDIS_CON0 0x0130
#define PMUCRU_SOFTRST_CON(n)   (PMUCRU_SOFTRST_CON0 + (n) * 4)

/* export related and operating SoC APIs */
void __dead2 soc_global_soft_reset(void);
void disable_dvfs_plls(void);
void disable_nodvfs_plls(void);
void enable_dvfs_plls(void);
void enable_nodvfs_plls(void);
void prepare_abpll_for_ddrctrl(void);
void restore_abpll(void);
void clk_gate_con_save(void);
void clk_gate_con_disable(void);
void clk_gate_con_restore(void);
void set_pmu_rsthold(void);
void pmu_sgrf_rst_hld(void);
__pmusramfunc void pmu_sgrf_rst_hld_release(void);
__pmusramfunc void restore_pmu_rsthold(void);
#endif /* SOC_H */
