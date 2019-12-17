/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_H__
#define __SOC_H__

#include <plat_private.h>

#ifndef BITS_WMSK
#define BITS_WMSK(msk, shift)	((msk) << (shift + REG_MSK_SHIFT))
#endif

enum plls_id {
	APLL_ID = 0,
	DPLL_ID,
	CPLL_ID,
	NPLL_ID,
	GPLL_ID,
	END_PLL_ID,
};

enum pll_mode {
	SLOW_MODE,
	NORM_MODE,
	DEEP_SLOW_MODE,
};

/***************************************************************************
 * GRF
 ***************************************************************************/
#define GRF_SOC_CON(i)		(0x0400 + (i) * 4)
#define GRF_PD_VO_CON0		0x0434
#define GRF_SOC_STATUS0		0x0480
#define GRF_CPU_STATUS0		0x0520
#define GRF_CPU_STATUS1		0x0524
#define GRF_SOC_NOC_CON0	0x0530
#define GRF_SOC_NOC_CON1	0x0534

#define CKECK_WFE_MSK		0x1
#define CKECK_WFI_MSK		0x10
#define CKECK_WFEI_MSK		0x11

#define GRF_SOC_CON2_NSWDT_RST_EN 12

/***************************************************************************
 * cru
 ***************************************************************************/
#define CRU_MODE		0xa0
#define CRU_MISC		0xa4
#define CRU_GLB_CNT_TH	0xb0
#define CRU_GLB_RST_ST	0xb4
#define CRU_GLB_SRST_FST	0xb8
#define CRU_GLB_SRST_SND	0xbc
#define CRU_GLB_RST_CON		0xc0

#define CRU_CLKSEL_CON		0x100
#define CRU_CLKSELS_CON(i)	(CRU_CLKSEL_CON + (i) * 4)
#define CRU_CLKSEL_CON_CNT	60

#define CRU_CLKGATE_CON		0x200
#define CRU_CLKGATES_CON(i)	(CRU_CLKGATE_CON + (i) * 4)
#define CRU_CLKGATES_CON_CNT	18

#define CRU_SOFTRST_CON		0x300
#define CRU_SOFTRSTS_CON(n)	(CRU_SOFTRST_CON + ((n) * 4))
#define CRU_SOFTRSTS_CON_CNT	12

#define CRU_AUTOCS_CON0(id)	(0x400 + (id) * 8)
#define CRU_AUTOCS_CON1(id)	(0x404 + (id) * 8)

#define CRU_CONS_GATEID(i)	(16 * (i))
#define GATE_ID(reg, bit)	((reg) * 16 + (bit))

#define CRU_GLB_SRST_FST_VALUE	0xfdb9
#define CRU_GLB_SRST_SND_VALUE	0xeca8

#define CRU_GLB_RST_TSADC_EXT 6
#define CRU_GLB_RST_WDT_EXT 7

#define CRU_GLB_CNT_RST_MSK  0xffff
#define CRU_GLB_CNT_RST_1MS  0x5DC0

#define CRU_GLB_RST_TSADC_FST BIT(0)
#define CRU_GLB_RST_WDT_FST BIT(1)

/***************************************************************************
 * pll
 ***************************************************************************/
#define CRU_PLL_CONS(id, i)	((id) * 0x20 + (i) * 4)
#define PLL_CON(i)		((i) * 4)
#define PLL_CON_CNT		5
#define PLL_LOCK_MSK		BIT(10)
#define PLL_MODE_SHIFT(id)	((id) == CPLL_ID ? \
				  2 : \
				  ((id) == DPLL_ID ? 4 : 2 * (id)))
#define PLL_MODE_MSK(id)	(0x3 << PLL_MODE_SHIFT(id))

#define PLL_LOCKED_TIMEOUT	600000U

/***************************************************************************
 * GPIO
 ***************************************************************************/
#define SWPORTA_DR		0x00
#define SWPORTA_DDR		0x04
#define GPIO_INTEN		0x30
#define GPIO_INT_STATUS		0x40
#define GPIO_NUMS		4

void clk_gate_con_save(uint32_t *clkgt_save);
void clk_gate_con_restore(uint32_t *clkgt_save);
void clk_gate_con_disable(void);

void px30_soc_reset_config(void);

#endif /* __SOC_H__ */
