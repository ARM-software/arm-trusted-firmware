/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2026, Rockchip Electronics Co., Ltd.
 */

#ifndef __SOC_H__
#define __SOC_H__

#define KHz				1000
#define MHz				(1000 * KHz)
#define OSC_HZ				(24 * MHz)

#define MCU_VALID_START_ADDRESS		0x800000

/* CRU */
#define GLB_SRST_FST_CFG_VAL		0xfdb9

#define CRU_PLLS_CON(pll_id, i)		((pll_id) * 0x20 + (i) * 0x4)
#define CRU_PLL_CON(i)			((i) * 0x4)
#define CRU_SSCGTBL_CON(i)		(0x140 + (i) * 0x4)
#define CRU_MODE_CON			0x280
#define CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define CRU_CLKSEL_CON_CNT		71
#define CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define CRU_CLKGATE_CON_CNT		16
#define CRU_SOFTRST_CON(i)		(0xa00 + (i) * 0x4)

#define CRU_GLB_CNT_TH			0xc00
#define CRU_GLB_RST_ST			0xc04
#define CRU_GLB_SRST_FST		0xc08
#define CRU_GLB_SRST_SND		0xc0c
#define CRU_GLB_RST_CON(i)		(0xc10 + (i) * 0x4)
#define CRU_GLB_RST_ST_NCLR		0xc20

#define CRU_PLLCON0_FBDIV_MASK		0xfff
#define CRU_PLLCON0_FBDIV_SHIFT		0
#define CRU_PLLCON0_POSTDIV1_MASK	0x7
#define CRU_PLLCON0_POSTDIV1_SHIFT	12
#define CRU_PLLCON0_BYPASS_SHIFT	15
#define CRU_PLLCON1_REFDIV_MASK		0x3f
#define CRU_PLLCON1_REFDIV_SHIFT	0
#define CRU_PLLCON1_POSTDIV2_MASK	0x7
#define CRU_PLLCON1_POSTDIV2_SHIFT	6
#define CRU_PLLCON1_LOCK_STATUS		BIT(10)
#define CRU_PLLCON1_DSMPD_MASK		0x1
#define CRU_PLLCON1_DSMPD_SHIFT		12
#define CRU_PLLCON1_PWRDOWN		BIT(13)

/* DDRCRU */
#define DDRCRU_CLKSEL_CON0		00x300
#define DDRCRU_CLKGATE_CON0		0x800
#define DDRCRU_SOFTRST_CON0		0xa00

/* PERI_CRU */
#define PERI_CRU_MODE_CON		0x280
#define PERI_CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define PERI_CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define PERI_CRU_CLKGATE_CON_CNT	2
#define PERI_CRU_SOFTRST_CON(i)		(0xa00 + (i) * 0x4)

/* BUS CRU */
#define BUS_CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define BUS_CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define BUS_CRU_CLKGATE_CON_CNT		7
#define BUS_CRU_SOFTRST_CON(i)		(0xa00 + (i) * 0x4)

/* BUS SCRU */
#define BUS_SCRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define BUS_SCRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define BUS_SCRU_CLKGATE_CON_CNT	3
#define BUS_SCRU_SOFTRST_CON(i)		(0xa00 + (i) * 0x4)

/* CORE CRU */
#define CORE_CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define CORE_CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define CORE_CRU_CLKGATE_CON_CNT	2

/* VI CRU */
#define VI_CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define VI_CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define VI_CRU_CLKGATE_CON_CNT		5

/* VEPU CRU */
#define VEPU_CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define VEPU_CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define VEPU_CRU_CLKGATE_CON_CNT	2

/* VCP CRU */
#define VCP_CRU_CLKSEL_CON(i)		(0x300 + (i) * 0x4)
#define VCP_CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define VCP_CRU_CLKGATE_CON_CNT		2

/* NPU CRU */
#define NPU_CRU_CLKSEL_CON0		0x300
#define NPU_CRU_CLKGATE_CON0		0x800
#define NPU_CRU_CLKGATE_CON_CNT		1

/* VDO CRU */
#define VDO_CRU_CLKSEL_CON0		0x300
#define VDO_CRU_CLKGATE_CON(i)		(0x800 + (i) * 0x4)
#define VDO_CRU_CLKGATE_CON_CNT		3

/* SYSGRF */
#define SYSGRF_BUSGRF_MISC		0xc
#define SYSGRF_NOC_CON(i)		(0x30 + (i) * 4)
#define SYSGRF_NOC_STATUS(i)		(0x100 + (i) * 4)
#define SYSGRF_SYS_STATUS		0x118

/* CPUGRF */
#define CPUGRF_CON(i)			((i) * 4)
#define CPUGRF_STATUS(i)		(0xc + (i) * 4)

/* DDRGRF */
#define DDRGRF_CON(i)			((i) * 4)
#define DDRGRF_STATUS(i)		(0x100 + (i) * 4)

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

#define OTP_CHIP_ID_OFF			0
#define OTP_FEA_ID_OFF			10
#define OTP_CHIP_ID_RM_OFF		14
#define OTP_FEA_ID_RM_OFF		22

#define SOC_RV1126B			0x52561126
#define SOC_UNKNOWN			0xeeee
#define SOC_ROOT			0x0
#endif /* __SOC_H__ */
