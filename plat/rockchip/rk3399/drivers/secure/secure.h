/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURE_H
#define SECURE_H

/**************************************************
 * sgrf reg, offset
 **************************************************/
#define SGRF_SOC_CON0_1(n)		(0xc000 + (n) * 4)
#define SGRF_SOC_CON3_7(n)		(0xe00c + ((n) - 3) * 4)
#define SGRF_SOC_CON8_15(n)		(0x8020 + ((n) - 8) * 4)
#define SGRF_SOC_CON(n) 		(n < 3 ? SGRF_SOC_CON0_1(n) :\
						(n < 8 ? SGRF_SOC_CON3_7(n) :\
							 SGRF_SOC_CON8_15(n)))

#define SGRF_PMU_SLV_CON0_1(n)		(0xc240 + ((n) - 0) * 4)
#define SGRF_SLV_SECURE_CON0_4(n)	(0xe3c0 + ((n) - 0) * 4)
#define SGRF_DDRRGN_CON0_16(n)		((n) * 4)
#define SGRF_DDRRGN_CON20_34(n)		(0x50 + ((n) - 20) * 4)

/* All of master in ns */
#define SGRF_SOC_ALLMST_NS		0xffff

/* security config for slave */
#define SGRF_SLV_S_WMSK			0xffff0000
#define SGRF_SLV_S_ALL_NS		0x0

/* security config pmu slave ip */
/* All of slaves  is ns */
#define SGRF_PMU_SLV_S_NS		BIT_WITH_WMSK(0)
/* slaves secure attr is configed */
#define SGRF_PMU_SLV_S_CFGED		WMSK_BIT(0)
#define SGRF_PMU_SLV_CRYPTO1_NS		WMSK_BIT(1)

#define SGRF_PMUSRAM_S			BIT(8)

#define SGRF_INTSRAM_S			BIT(13)

/* ddr region */
#define SGRF_DDR_RGN_0_16_WMSK		0x0fff  /* DDR RGN 0~16 size mask */

#define SGRF_DDR_RGN_DPLL_CLK		BIT_WITH_WMSK(15) /* DDR PLL output clock */
#define SGRF_DDR_RGN_RTC_CLK		BIT_WITH_WMSK(14) /* 32K clock for DDR PLL */

/* All security of the DDR RGNs are bypass */
#define SGRF_DDR_RGN_BYPS		BIT_WITH_WMSK(9)
/* All security of the DDR RGNs are not bypass */
#define SGRF_DDR_RGN_NO_BYPS		WMSK_BIT(9)

/* The MST access the ddr rgn n with secure attribution */
#define SGRF_L_MST_S_DDR_RGN(n)		BIT_WITH_WMSK((n))
/* bits[16:8]*/
#define SGRF_H_MST_S_DDR_RGN(n)		BIT_WITH_WMSK((n) + 8)

#define SGRF_PMU_CON0			0x0c100
#define SGRF_PMU_CON(n)   		(SGRF_PMU_CON0 + (n) * 4)

/**************************************************
 * secure timer
 **************************************************/
/* chanal0~5 */
#define STIMER0_CHN_BASE(n)		(STIME_BASE + 0x20 * (n))
/* chanal6~11 */
#define STIMER1_CHN_BASE(n)		(STIME_BASE + 0x8000 + 0x20 * (n))

 /* low 32 bits */
#define TIMER_END_COUNT0		0x00
 /* high 32 bits */
#define TIMER_END_COUNT1		0x04

#define TIMER_CURRENT_VALUE0		0x08
#define TIMER_CURRENT_VALUE1		0x0C

 /* low 32 bits */
#define TIMER_INIT_COUNT0		0x10
 /* high 32 bits */
#define TIMER_INIT_COUNT1		0x14

#define TIMER_INTSTATUS			0x18
#define TIMER_CONTROL_REG		0x1c

#define TIMER_EN			0x1

#define TIMER_FMODE			(0x0 << 1)
#define TIMER_RMODE			(0x1 << 1)

/**************************************************
 * secure WDT
 **************************************************/
#define PCLK_WDT_CA53_GATE_SHIFT	8
#define PCLK_WDT_CM0_GATE_SHIFT		10

/* export secure operating APIs */
void secure_watchdog_gate(void);
__pmusramfunc void secure_watchdog_ungate(void);
void secure_timer_init(void);
void secure_sgrf_init(void);
void secure_sgrf_ddr_rgn_init(void);
__pmusramfunc void sram_secure_timer_init(void);

#endif /* SECURE_H */
