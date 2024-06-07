/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_H__
#define __SOC_H__

#define RKFPGA_DEV_RNG0_BASE		0xf8000000
#define RKFPGA_DEV_RNG0_SIZE		0x07fff000

#define CRU_MODE_CON00			0x00c0
#define PMUCRU_MODE_CON00		0x0080

#define CRU_GLB_SRST_FST		0x00d4
#define GLB_SRST_FST_CFG_VAL		0xfdb9

#define PMU_GRF_GPIO0A_IOMUX_L		0x00
#define PMU_GRF_SOC_CON(i)		(0x0100 + i * 4)

#define CRU_SOFTRST_CON			0x300
#define CRU_SOFTRSTS_CON(n)		(CRU_SOFTRST_CON + ((n) * 4))
#define CRU_SOFTRSTS_CON_CNT		26
#define GRF_DDR_CON3			0x000c
#define SGRF_FIREWALL_SLV_CON(i)	(0x240 + i * 4)

#define FIREWALL_DDR_FW_DDR_CON_REG	0x80

 /* low 32 bits */
#define TIMER_LOAD_COUNT0		0x00
#define TIMER_LOAD_COUNT1		0x04
#define TIMER_CURRENT_VALUE0		0x08
#define TIMER_CURRENT_VALUE1		0x0c
#define TIMER_CONTROL_REG		0x10
#define TIMER_INTSTATUS			0x18
#define TIMER_DIS			0x0
#define TIMER_EN			0x1
#define STIMER0_CHN_BASE(n)		(STIME_BASE + 0x20 * (n))

#define PMU_GRF_GPIO0B_IOMUX_L		0x0008
#define PMUCRU_PMUCLKSEL_CON00		0x0100
#define PMUPVTM_BASE			0xfdd80000
#define PVTM_CON0			0x0004
#define PVTM_CON1			0x0008
#define PVTM_STATUS0			0x0080
#define PVTM_STATUS1			0x0084
#define PMUCRU_PMUGATE_CON01		0x0184
#define PVTM_CALC_CNT			0x200
#define PMU_GRF_DLL_CON0		0x0180

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

#endif /* __SOC_H__ */
