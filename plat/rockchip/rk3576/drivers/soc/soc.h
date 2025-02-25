/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#ifndef __SOC_H__
#define __SOC_H__

enum pll_id {
	APLL_ID,
	CPLL_ID,
	DPLL_ID,
	GPLL_ID,
};

enum cru_mode_con00 {
	CLK_APLL,
	CLK_CPLL,
	CLK_GPLL,
	CLK_DPLL,
};

#define KHz				1000
#define MHz				(1000 * KHz)
#define OSC_HZ				(24 * MHz)

#define MCU_VALID_START_ADDRESS		0x800000

/* CRU */
#define GLB_SRST_FST_CFG_VAL		0xfdb9

#define CRU_PLLS_CON(pll_id, i)		((pll_id) * 0x20 + (i) * 0x4)
#define CRU_PLL_CON(i)			((i) * 0x4)
#define CRU_MODE_CON			0x280
#define CRU_CLKSEL_CON(i)		((i) * 0x4 + 0x300)
#define CRU_CLKSEL_CON_CNT		181
#define CRU_CLKGATE_CON(i)		((i) * 0x4 + 0x800)
#define CRU_CLKGATE_CON_CNT		80
#define CRU_SOFTRST_CON(i)		((i) * 0x4 + 0xa00)
#define CRU_SOFTRST_CON_CNT		80

#define CRU_GLB_CNT_TH			0xc00
#define CRU_GLB_RST_ST			0xc04
#define CRU_GLB_SRST_FST		0xc08
#define CRU_GLB_SRST_SND		0xc0c
#define CRU_GLB_RST_CON			0xc10
#define CRU_GLB_RST_ST_NCLR		0xc14
#define CRU_LITCOREWFI_CON0		0xc40
#define CRU_BIGCOREWFI_CON0		0xc44
#define CRU_NON_SECURE_GT_CON0		0xc48

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

/* LCORE_CRU */
#define LCORE_CRU_MODE_CON		0x280
#define LCORE_CRU_CLKSEL_CON(i)		((i) * 0x4 + 0x300)
#define LCORE_CRU_CLKSEL_CON_CNT	4
#define LCORE_CRU_CLKGATE_CON(i)	((i) * 0x4 + 0x800)
#define LCORE_CRU_CLKGATE_CON_CNT	2
#define LCORE_CRU_SOFTRST_CON(i)	((i) * 0x4 + 0xa00)
#define LCORE_CRU_SOFTRST_CON_CNT	4

/* BCORE_CRU */
#define BCORE_CRU_MODE_CON		0x280
#define BCORE_CRU_CLKSEL_CON(i)		((i) * 0x4 + 0x300)
#define BCORE_CRU_CLKSEL_CON_CNT	5
#define BCORE_CRU_CLKGATE_CON(i)	((i) * 0x4 + 0x800)
#define BCORE_CRU_CLKGATE_CON_CNT	3
#define BCORE_CRU_SOFTRST_CON(i)	((i) * 0x4 + 0xa00)
#define BCORE_CRU_SOFTRST_CON_CNT	4

/* DDRCRU */
#define DDRCRU_MODE_CON			0x280
#define DDRCRU_CLKSEL_CON(i)		((i) * 0x4 + 0x300)
#define DDRCRU_CLKGATE_CON(i)		((i) * 0x4 + 0x800)
#define DDRCRU_SOFTRST_CON(i)		((i) * 0x4 + 0xa00)

/* CCICRU */
#define CCICRU_MODE_CON			0x280
#define CCICRU_CLKSEL_CON(i)		((i) * 0x4 + 0x300)
#define CCICRU_CLKSEL_CON_CNT		10
#define CCICRU_CLKGATE_CON(i)		((i) * 0x4 + 0x800)
#define CCICRU_CLKGATE_CON_CNT		7
#define CCICRU_SOFTRST_CON(i)		((i) * 0x4 + 0xa00)
#define CCICRU_SOFTRST_CON_CNT		7

/* CRU AUTOCS */
#define CRU_AUTOCS_CON(offset)		(CRU_BASE + (offset))
#define CRU_AUTOCS_SEC_CON(offset)	(SECURE_CRU_BASE + (offset))
#define CRU_AUTOCS_CCI_CON(offset)	(CCI_CRU_BASE + (offset))
#define AUTOCS_EN_BIT			BIT(12)

/* PHP_CRU */
#define PHP_CRU_MODE_CON		0x280
#define PHP_CRU_CLKSEL_CON(i)		((i) * 0x4 + 0x300)
#define PHP_CRU_CLKSEL_CON_CNT		2
#define PHP_CRU_CLKGATE_CON(i)		((i) * 0x4 + 0x800)
#define PHP_CRU_CLKGATE_CON_CNT		2
#define PHP_CRU_SOFTRST_CON(i)		((i) * 0x4 + 0xa00)
#define PHP_CRU_SOFTRST_CON_CNT		2

/* SECURE CRU */
#define SECURE_CRU_CLKSEL_CON(i)	((i) * 0x4 + 0x300)
#define SECURE_CRU_CLKSEL_CON_CNT	1
#define SECURE_CRU_CLKGATE_CON(i)	((i) * 0x4 + 0x800)
#define SECURE_CRU_CLKGATE_CON_CNT	1
#define SECURE_CRU_SOFTRST_CON(i)	((i) * 0x4 + 0xa00)
#define SECURE_CRU_SOFTRST_CON_CNT	1

/* SECURE SCRU */
#define SECURE_SCRU_CLKSEL_CON(i)	((i) * 0x4 + 0x4000)
#define SECURE_SCRU_CLKSEL_CON_CNT	7
#define SECURE_SCRU_CLKGATE_CON(i)	((i) * 0x4 + 0x4028)
#define SECURE_SCRU_CLKGATE_CON_CNT	6
#define SECURE_SCRU_SOFTRST_CON(i)	((i) * 0x4 + 0x4050)
#define SECURE_SCRU_SOFTRST_CON_CNT	6
#define SECURE_SCRU_MODE_CON		0x4280

/* SYSGRF */
#define SYSGRF_SOC_CON(i)		((i) * 4)
#define SYSGRF_SOC_STATUS		0x30
#define SYSGRF_NOC_CON(i)		(0x40 + (i) * 4)
#define SYSGRF_NOC_STATUS(i)		(0x60 + (i) * 4)
#define SYSGRF_MEM_CON(i)		(0x80 + (i) * 4)
#define SYSGRF_STATUS0			0x140
#define SYSGRF_STATUS1			0x144

/* COREGRF */
#define COREGRF_SOC_STATUS(i)		(0x2c + (i) * 4)
#define COREGRF_CPU_CON(i)		(0x34 + (i) * 4)

/* DDRGRF */
#define DDRGRF_CHA_CON(i)		((i) * 4)
#define DDRGRF_CHB_CON(i)		(0x100 + (i) * 4)
#define DDRGRF_CHA_ST(i)		(0x60 + (i) * 4)
#define DDRGRF_CHB_ST(i)		(0xb0 + (i) * 4)
#define DDRGRF_CON(i)			(0x140 + (i) * 4)

/* CCIGRF */
#define CCIGRF_CON(i)			((i) * 4)
#define CCIGRF_STATUS(i)		(0x34 + (i) * 4)

/* IOC */
#define VCCIO_IOC_MISC_CON(i)		(0x400 + (i) * 4)

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

/* WDT */
#define WDT_CR			0x0
#define WDT_TORR		0x4
#define WDT_CCVR		0x8
#define WDT_CRR			0xc
#define WDT_STAT		0x10
#define WDT_EOI			0x14

#define WDT_EN			BIT(0)
#define WDT_RSP_MODE		BIT(1)

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

/* hp timer */
#define TIMER_HP_REVISION		0x00
#define TIMER_HP_CTRL			0x04
#define TIMER_HP_INTR_EN		0x08
#define TIMER_HP_T24_GCD		0x0c
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
#define TIMER_HP_UPD_EN			0x50

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
#define GPIO_STORE_ST_L			0x0080
#define GPIO_STORE_ST_H			0x0084
#define GPIO_REG_GROUP_L		0x0100
#define GPIO_REG_GROUP_H		0x0104
#define GPIO_VIRTUAL_EN			0x0108
#define GPIO_REG_GROUP1_L		0x0110
#define GPIO_REG_GROUP1_H		0x0114
#define GPIO_REG_GROUP2_L		0x0118
#define GPIO_REG_GROUP2_H		0x011c
#define GPIO_REG_GROUP3_L		0x0120
#define GPIO_REG_GROUP3_H		0x0124

/* PWM */
#define PMW_PWRCAPTURE_VAL		0x15c

#define SOC_RK3576A1			0x35760101
#define SOC_RK3576J1			0x35760a01
#define SOC_RK3576M1			0x35760d01
#define SOC_RK3576S1			0x35761301
#define SOC_UNKNOWN			0xeeee
#define SOC_ROOT			0x0

int rk_soc_is_(uint32_t soc_id);
uint32_t timer_hp_get_freq(void);
int soc_bus_div_sip_handler(uint32_t id, uint32_t cfg, uint32_t enable_msk);
void autocs_suspend(void);
void autocs_resume(void);

#endif /* __SOC_H__ */
