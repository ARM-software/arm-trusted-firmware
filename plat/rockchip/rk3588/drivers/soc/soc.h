/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_H__
#define __SOC_H__

enum pll_id {
	APLL_ID,
	DPLL_ID,
	GPLL_ID,
	CPLL_ID,
	NPLL_ID,
	VPLL_ID,
};

enum pmu_pll_id {
	PPLL_ID = 0,
	HPLL_ID
};

enum cru_mode_con00 {
	CLK_APLL,
	CLK_DPLL,
	CLK_CPLL,
	CLK_GPLL,
	CLK_REVSERVED,
	CLK_NPLL,
	CLK_VPLL,
	CLK_USBPLL,
};

#define KHz				1000
#define MHz				(1000 * KHz)
#define OSC_HZ				(24 * MHz)

/* CRU */
#define GLB_SRST_FST_CFG_VAL		0xfdb9

#define CRU_PLLS_CON(pll_id, i)		(0x160 + (pll_id) * 0x20 + (i) * 0x4)
#define CRU_PLL_CON(i)			((i) * 0x4)
#define CRU_MODE_CON0			0x280
#define CRU_CLKSEL_CON(i)		((i) * 0x4 + 0x300)
#define CRU_CLKGATE_CON(i)		((i) * 0x4 + 0x800)
#define CRU_CLKGATE_CON_CNT		78
#define CRU_SOFTRST_CON(i)		((i) * 0x4 + 0xa00)
#define CRU_GLB_CNT_TH			0xc00
#define CRU_GLB_SRST_FST		0xc08
#define CRU_GLB_SRST_SND		0xc0c
#define CRU_GLB_RST_CON			0xc10
#define CRU_GLB_RST_ST			0xc04
#define CRU_SDIO_CON0			0xc24
#define CRU_SDIO_CON1			0xc28
#define CRU_SDMMC_CON0			0xc30
#define CRU_SDMMC_CON1			0xc34
#define CRU_AUTOCS_CON0(id)		(0xd00 + (id) * 8)
#define CRU_AUTOCS_CON1(id)		(0xd04 + (id) * 8)

#define CRU_AUTOCS_ID_CNT		74

#define CRU_PLLCON0_M_MASK		0x3ff
#define CRU_PLLCON0_M_SHIFT		0
#define CRU_PLLCON1_P_MASK		0x3f
#define CRU_PLLCON1_P_SHIFT		0
#define CRU_PLLCON1_S_MASK		0x7
#define CRU_PLLCON1_S_SHIFT		6
#define CRU_PLLCON2_K_MASK		0xffff
#define CRU_PLLCON2_K_SHIFT		0
#define CRU_PLLCON1_PWRDOWN		BIT(13)
#define CRU_PLLCON6_LOCK_STATUS		BIT(15)

#define CRU_BIGCPU02_RST_MSK		0x30
#define CRU_BIGCPU13_RST_MSK		0x300

#define PHPCRU_CLKGATE_CON		0x800
#define PHPCRU_CLKGATE_CON_CNT		1

#define SECURECRU_CLKGATE_CON(i)	((i) * 0x4 + 0x800)
#define SECURECRU_CLKGATE_CON_CNT	4

#define PMU1CRU_CLKGATE_CON_CNT		6

/* CENTER GRF */
#define CENTER_GRF_CON(i)		((i) * 4)

/* PMU1GRF */
#define PMU1GRF_SOC_CON(n)		((n) * 4)
#define PMU1GRF_SOC_ST			0x60
#define PMU1GRF_OS_REG(n)		(0x200 + ((n) * 4))

#define PMU_MCU_HALT			BIT(7)
#define PMU_MCU_SLEEP			BIT(9)
#define PMU_MCU_DEEPSLEEP		BIT(10)
#define PMU_MCU_STOP_MSK		\
	(PMU_MCU_HALT | PMU_MCU_SLEEP | PMU_MCU_DEEPSLEEP)

/* SYSGRF */
#define SYS_GRF_NOC_CON(n)		(0x100 + (n) * 4)
#define SYS_GRF_SOC_CON(n)		(0x300 + (n) * 4)
#define SYS_GRF_SOC_STATUS(n)		(0x380 + (n) * 4)

#define SYS_GRF_LITTLE_CPUS_WFE		0xf
#define SYS_GRF_CORE0_CPUS_WFE		0x30
#define SYS_GRF_CORE1_CPUS_WFE		0xc0
#define SYS_GRF_BIG_CPUS_WFE		0xf0
#define SYS_GRF_LITTLE_CPUS_WFI		0xf00
#define SYS_GRF_CORE0_CPUS_WFI		0x3000
#define SYS_GRF_CORE1_CPUS_WFI		0xc000

/* pvtm */
#define PVTM_CON(i)			(0x4 + (i) * 4)
#define PVTM_INTEN			0x70
#define PVTM_INTSTS			0x74
#define PVTM_STATUS(i)			(0x80 + (i) * 4)
#define PVTM_CALC_CNT			0x200

enum pvtm_con0 {
	pvtm_start = 0,
	pvtm_osc_en = 1,
	pvtm_osc_sel = 2,
	pvtm_rnd_seed_en = 5,
};

/* timer */
#define TIMER_LOAD_COUNT0		0x00
#define TIMER_LOAD_COUNT1		0x04
#define TIMER_CURRENT_VALUE0		0x08
#define TIMER_CURRENT_VALUE1		0x0c
#define TIMER_CONTROL_REG		0x10
#define TIMER_INTSTATUS			0x18

#define TIMER_DIS			0x0
#define TIMER_EN			0x1

#define TIMER_FMODE			(0x0 << 1)
#define TIMER_RMODE			(0x1 << 1)

#define STIMER0_CHN_BASE(n)		(STIMER0_BASE + 0x20 * (n))
#define STIMER1_CHN_BASE(n)		(STIMER1_BASE + 0x20 * (n))

/* cpu timer */
#define TIMER_HP_REVISION		0x0
#define TIMER_HP_CTRL			0x4
#define TIMER_HP_INT_EN			0x8
#define TIMER_HP_T24_GCD		0xc
#define TIMER_HP_T32_GCD		0x10
#define TIMER_HP_LOAD_COUNT0		0x14
#define TIMER_HP_LOAD_COUNT1		0x18
#define TIMER_HP_T24_DELAT_COUNT0	0x1c
#define TIMER_HP_T24_DELAT_COUNT1	0x20
#define TIMER_HP_CURR_32K_VALUE0	0x24
#define TIMER_HP_CURR_32K_VALUE1	0x28
#define TIMER_HP_CURR_TIMER_VALUE0	0x2c
#define TIMER_HP_CURR_TIMER_VALUE1	0x30
#define TIMER_HP_T24_32BEGIN0		0x34
#define TIMER_HP_T24_32BEGIN1		0x38
#define TIMER_HP_T32_24END0		0x3c
#define TIMER_HP_T32_24END1		0x40
#define TIMER_HP_BEGIN_END_VALID	0x44
#define TIMER_HP_SYNC_REQ		0x48
#define TIMER_HP_INTR_STATUS		0x4c

 /* GPIO */
#define GPIO_SWPORT_DR_L		0x0000
#define GPIO_SWPORT_DR_H		0x0004
#define GPIO_SWPORT_DDR_L		0x0008
#define GPIO_SWPORT_DDR_H		0x000c
#define GPIO_INT_EN_L			0x0010
#define GPIO_INT_EN_H			0x0014
#define GPIO_INT_MASK_L			0x0018
#define GPIO_INT_MASK_H			0x001c
#define GPIO_INT_TYPE_L			0x0020
#define GPIO_INT_TYPE_H			0x0024
#define GPIO_INT_POLARITY_L		0x0028
#define GPIO_INT_POLARITY_H		0x002c
#define GPIO_INT_BOTHEDGE_L		0x0030
#define GPIO_INT_BOTHEDGE_H		0x0034
#define GPIO_DEBOUNCE_L			0x0038
#define GPIO_DEBOUNCE_H			0x003c
#define GPIO_DBCLK_DIV_EN_L		0x0040
#define GPIO_DBCLK_DIV_EN_H		0x0044
#define GPIO_DBCLK_DIV_CON		0x0048
#define GPIO_INT_STATUS			0x0050
#define GPIO_INT_RAWSTATUS		0x0058
#define GPIO_PORT_EOI_L			0x0060
#define GPIO_PORT_EOI_H			0x0064
#define GPIO_EXT_PORT			0x0070
#define GPIO_VER_ID			0x0078

/* DDRGRF */
#define DDRGRF_CHA_CON(i)		((i) * 4)
#define DDRGRF_CHB_CON(i)		(0x30 + (i) * 4)

#define DDR_CHN_CNT			4

#endif /* __SOC_H__ */
