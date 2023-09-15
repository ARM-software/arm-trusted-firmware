/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#ifndef _DT_BINDINGS_CLOCK_STM32MP25_CLKSRC_H_
#define _DT_BINDINGS_CLOCK_STM32MP25_CLKSRC_H_

#define CMD_DIV		0
#define CMD_MUX		1
#define CMD_CLK		2
#define CMD_FLEXGEN	3

#define CMD_ADDR_BIT	0x80000000

#define CMD_SHIFT	26
#define CMD_MASK	0xFC000000
#define CMD_DATA_MASK	0x03FFFFFF

#define DIV_ID_SHIFT	8
#define DIV_ID_MASK	0x0000FF00

#define DIV_DIVN_SHIFT	0
#define DIV_DIVN_MASK	0x000000FF

#define MUX_ID_SHIFT	4
#define MUX_ID_MASK	0x00000FF0

#define MUX_SEL_SHIFT	0
#define MUX_SEL_MASK	0x0000000F

/* CLK define */
#define CLK_ON_MASK	BIT(21)
#define CLK_ON_SHIFT	21

#define CLK_ID_MASK	GENMASK_32(20, 12)
#define CLK_ID_SHIFT	12

#define CLK_NO_DIV_MASK	0x0000080
#define CLK_DIV_MASK	GENMASK_32(10, 5)
#define CLK_DIV_SHIFT	5

#define CLK_NO_SEL_MASK	0x00000010
#define CLK_SEL_MASK	GENMASK_32(3, 0)
#define CLK_SEL_SHIFT	0

#define CLK_CFG(clk_id, sel, div, state)	((CMD_CLK << CMD_SHIFT) |\
						 ((state) << CLK_ON_SHIFT) |\
						 ((clk_id) << CLK_ID_SHIFT) |\
						 ((div) << CLK_DIV_SHIFT) |\
						 ((sel) << CLK_SEL_SHIFT))

#define CLK_OFF		0
#define CLK_ON		1
#define CLK_NODIV	0x00000040
#define CLK_NOMUX	0x00000010

/* Flexgen define */
#define FLEX_ID_SHIFT	13
#define FLEX_SEL_SHIFT	9
#define FLEX_PDIV_SHIFT	6
#define FLEX_FDIV_SHIFT	0

#define FLEX_ID_MASK	GENMASK_32(18, 13)
#define FLEX_SEL_MASK	GENMASK_32(12, 9)
#define FLEX_PDIV_MASK	GENMASK_32(8, 6)
#define FLEX_FDIV_MASK	GENMASK_32(5, 0)

#define DIV_CFG(div_id, div)	((CMD_DIV << CMD_SHIFT) |\
				 ((div_id) << DIV_ID_SHIFT |\
				 (div)))

#define MUX_CFG(mux_id, sel)	((CMD_MUX << CMD_SHIFT) |\
				 ((mux_id) << MUX_ID_SHIFT |\
				 (sel)))

#define CLK_ADDR_SHIFT		16
#define CLK_ADDR_MASK		0x7FFF0000
#define CLK_ADDR_VAL_MASK	0xFFFF

#define DIV_LSMCU	0
#define DIV_APB1	1
#define DIV_APB2	2
#define DIV_APB3	3
#define DIV_APB4	4
#define DIV_APBDBG	5
#define DIV_RTC		6
#define DIV_NB		7

#define MUX_MUXSEL0	0
#define MUX_MUXSEL1	1
#define MUX_MUXSEL2	2
#define MUX_MUXSEL3	3
#define MUX_MUXSEL4	4
#define MUX_MUXSEL5	5
#define MUX_MUXSEL6	6
#define MUX_MUXSEL7	7
#define MUX_XBARSEL	8
#define MUX_RTC		9
#define MUX_MCO1	10
#define MUX_MCO2	11
#define MUX_ADC12	12
#define MUX_ADC3	13
#define MUX_USB2PHY1	14
#define MUX_USB2PHY2	15
#define MUX_USB3PCIEPHY	16
#define MUX_DSIBLANE	17
#define MUX_DSIPHY	18
#define MUX_LVDSPHY	19
#define MUX_DTS		20
#define MUX_CPU1	21
#define MUX_D3PER	22
#define MUX_NB		23

#define MUXSEL_HSI		0
#define MUXSEL_HSE		1
#define MUXSEL_MSI		2

/* KERNEL source clocks */
#define MUX_RTC_DISABLED	0x0
#define MUX_RTC_LSE		0x1
#define MUX_RTC_LSI		0x2
#define MUX_RTC_HSE		0x3

#define MUX_MCO1_FLEX61		0x0
#define MUX_MCO1_OBSER0		0x1

#define MUX_MCO2_FLEX62		0x0
#define MUX_MCO2_OBSER1		0x1

#define MUX_ADC12_FLEX46	0x0
#define MUX_ADC12_LSMCU		0x1

#define MUX_ADC3_FLEX47		0x0
#define MUX_ADC3_LSMCU		0x1
#define MUX_ADC3_FLEX46		0x2

#define MUX_USB2PHY1_FLEX57	0x0
#define MUX_USB2PHY1_HSE	0x1

#define MUX_USB2PHY2_FLEX58	0x0
#define MUX_USB2PHY2_HSE	0x1

#define MUX_USB3PCIEPHY_FLEX34	0x0
#define MUX_USB3PCIEPHY_HSE	0x1

#define MUX_DSIBLANE_FLEX28	0x0
#define MUX_DSIBLANE_FLEX27	0x1

#define MUX_DSIPHY_FLEX28	0x0
#define MUX_DSIPHY_HSE		0x1

#define MUX_LVDSPHY_FLEX32	0x0
#define MUX_LVDSPHY_HSE		0x1

#define MUX_DTS_HSI		0x0
#define MUX_DTS_HSE		0x1
#define MUX_DTS_MSI		0x2

#define MUX_D3PER_MSI		0x0
#define MUX_D3PER_LSI		0x1
#define MUX_D3PER_LSE		0x2

/* PLLs source clocks */
#define PLL_SRC_HSI		0x0
#define PLL_SRC_HSE		0x1
#define PLL_SRC_MSI		0x2
#define PLL_SRC_DISABLED	0x3

/* XBAR source clocks */
#define XBAR_SRC_PLL4		0x0
#define XBAR_SRC_PLL5		0x1
#define XBAR_SRC_PLL6		0x2
#define XBAR_SRC_PLL7		0x3
#define XBAR_SRC_PLL8		0x4
#define XBAR_SRC_HSI		0x5
#define XBAR_SRC_HSE		0x6
#define XBAR_SRC_MSI		0x7
#define XBAR_SRC_HSI_KER	0x8
#define XBAR_SRC_HSE_KER	0x9
#define XBAR_SRC_MSI_KER	0xA
#define XBAR_SRC_SPDIF_SYMB	0xB
#define XBAR_SRC_I2S		0xC
#define XBAR_SRC_LSI		0xD
#define XBAR_SRC_LSE		0xE

/*
 * Configure a XBAR channel with its clock source
 * channel_nb: XBAR channel number from 0 to 63
 * channel_src: one of the 15 previous XBAR source clocks defines
 * channel_prediv: value of the PREDIV in channel RCC_PREDIVxCFGR register
 *		   can be either 1, 2, 4 or 1024
 * channel_findiv: value of the FINDIV in channel RCC_FINDIVxCFGR register
 *		   from 1 to 64
 */

#define FLEXGEN_CFG(ch, sel, pdiv, fdiv)	((CMD_FLEXGEN << CMD_SHIFT) |\
						((ch) << FLEX_ID_SHIFT) |\
						((sel) << FLEX_SEL_SHIFT) |\
						((pdiv) << FLEX_PDIV_SHIFT) |\
						((fdiv) << FLEX_FDIV_SHIFT))

/* Register addresses of MCO1 & MCO2 */
#define MCO1			0x494
#define MCO2			0x498

#define MCO_OFF			0
#define MCO_ON			1
#define MCO_STATUS_SHIFT	8

#define MCO_CFG(addr, sel, status)	(CMD_ADDR_BIT |\
					((addr) << CLK_ADDR_SHIFT) |\
					((status) << MCO_STATUS_SHIFT) |\
					(sel))

/* define for st,pll /csg */
#define SSCG_MODE_CENTER_SPREAD	0
#define SSCG_MODE_DOWN_SPREAD	1

/* define for st,drive */
#define LSEDRV_LOWEST		0
#define LSEDRV_MEDIUM_LOW	1
#define LSEDRV_MEDIUM_HIGH	2
#define LSEDRV_HIGHEST		3

#endif /* _DT_BINDINGS_CLOCK_STM32MP25_CLKSRC_H_ */
