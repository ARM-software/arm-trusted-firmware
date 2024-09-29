/*
 * Copyright (c) 2024-2025, Rockchip Electronics Co., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RK3568_CLOCK_H
#define RK3568_CLOCK_H

#define CRU_CLKGATE_CON0		0x300
#define CRU_CLKGATES_CON(i)		(CRU_CLKGATE_CON0 + (i) * 4)

#define SCRU_GATE_CON00			0x0180
#define SCRU_GATE_CON01			0x0184
#define SCRU_GATE_CON02			0x0188

#define SGRF_SOC_CON2			0x0008

#define MPLL_HZ				(800 * MHz)
#define MPLL_REFDIV			3
#define MPLL_FBDIV			200
#define MPLL_POSTDIV1			2
#define MPLL_POSTDIV2			1
#define MPLL_DSMPD			1
#define MPLL_FRAC			0

#define PPLL_HZ				(200 * MHz)
#define GPLL_HZ				(1188 * MHz)
#define ACLK_SECURE_FLASH_S_HZ		(297 * MHz)
#define HCLK_SECURE_FLASH_S_HZ		(148500000)
#define PCLK_SECURE_FLASH_S_HZ		(99 * MHz)
#define PCLK_TOP_S_HZ			(99 * MHz)
#define HCLK_VO_S_HZ			(99 * MHz)
#define PCLK_DDR_HZ			(99 * MHz)
#define PCLK_PDPMU_S_HZ			(100 * MHz)
#define HCLK_PDPMU_S_HZ			(100 * MHz)

#define RK3568_PLL_MODE_CON		0x20
#define RK3568_PLL_MODE_SHIFT		0
#define RK3568_PLL_MODE_MASK		(0x3 << RK3568_PLL_MODE_SHIFT)
#define RK3568_PLL_MODE_SLOWMODE	0
#define RK3568_PLL_MODE_NORMAL		1
#define RK3568_PLLCON(i)		(i * 0x4)
#define RK3568_PLLCON0_FBDIV_MASK	0xfff
#define RK3568_PLLCON0_FBDIV_SHIFT	0
#define RK3568_PLLCON0_POSTDIV1_MASK	0x7
#define RK3568_PLLCON0_POSTDIV1_SHIFT	12
#define RK3568_PLLCON1_REFDIV_MASK	0x3f
#define RK3568_PLLCON1_REFDIV_SHIFT	0
#define RK3568_PLLCON1_POSTDIV2_MASK	0x7
#define RK3568_PLLCON1_POSTDIV2_SHIFT	6
#define RK3568_PLLCON1_LOCK_STATUS	(1 << 10)
#define RK3568_PLLCON1_DSMPD_MASK	0x1
#define RK3568_PLLCON1_DSMPD_SHIFT	12
#define RK3568_PLLCON1_PWRDOWN		(1 << 13)

#define RK3568_CLK_SEL(x)		((x) * 0x4 + 0x100)
#define RK3568_PMUCLK_SEL(x)		((x) * 0x4 + 0x100)

#define PMUCRU_MODE_CON00		0x0080
#define PMUCRU_PMUCLKSEL_CON00		0x0100
#define PMUCRU_PMUCLKSEL_CON03		0x010c
#define PMUCRU_PMUGATE_CON01		0x0184
#define PMUCRU_CLKGATES_CON(i)		(0x180 + (i) * 4)
#define PMUCRU_CLKGATES_CON_CNT		3

void pvtplls_suspend(void);
void pvtplls_resume(void);
void rockchip_clock_init(void);

#endif /* RK3568_CLOCK_H */
